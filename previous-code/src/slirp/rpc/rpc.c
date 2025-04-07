/*
 * Remote Procedure Call
 * 
 * Created by Simon Schubiger
 * Rewritten in C by Andreas Grabher
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <slirp.h>
#include <stdlib.h>

#include "rpc.h"
#include "portmap.h"
#include "mount.h"
#include "nfs.h"
#include "bootparam.h"
#include "netinfobind.h"
#include "csocket.h"
#include "udpsocket.h"
#include "tcpsocket.h"
#include "dns.h"

#include "configuration.h"


#define DBG 0

struct ft_t* nfsd_fts[1];

struct rpc_prog_t* rpc_prog_list;

const struct rpc_prog_t rpc_prog_table_template[] = 
{
    { BOOTPARAMPROG, BOOTPARAMVERS, IPPROTO_UDP, 0,        bootparam_prog, 1, "BOOTPARAM"  , NULL, NULL, NULL },
    { BOOTPARAMPROG, BOOTPARAMVERS, IPPROTO_TCP, 0,        bootparam_prog, 1, "BOOTPARAM"  , NULL, NULL, NULL },
    { MOUNTPROG,     MOUNTVERS,     IPPROTO_UDP, 0,        mount_prog,     1, "MOUNT"      , NULL, NULL, NULL },
    { MOUNTPROG,     MOUNTVERS,     IPPROTO_TCP, 0,        mount_prog,     1, "MOUNT"      , NULL, NULL, NULL },
    { PORTMAPPROG,   PORTMAPVERS,   IPPROTO_UDP, PORT_RPC, portmap_prog,   1, "PORTMAP"    , NULL, NULL, NULL },
    { PORTMAPPROG,   PORTMAPVERS,   IPPROTO_TCP, PORT_RPC, portmap_prog,   1, "PORTMAP"    , NULL, NULL, NULL },
    { NFSPROG,       NFSVERS,       IPPROTO_UDP, PORT_NFS, nfs_prog,       1, "NFS"        , NULL, NULL, NULL },
    { NFSPROG,       NFSVERS,       IPPROTO_TCP, PORT_NFS, nfs_prog,       1, "NFS"        , NULL, NULL, NULL },
    { NIBINDPROG,    NIBINDVERS,    IPPROTO_UDP, 0,        nibind_prog,    1, "NETINFOBIND", NULL, NULL, NULL },
    { NIBINDPROG,    NIBINDVERS,    IPPROTO_TCP, 0,        nibind_prog,    1, "NETINFOBIND", NULL, NULL, NULL }
};


static void rpc_set_cred(struct rpc_t* rpc) {
    struct auth_unix_t* auth = (struct auth_unix_t*)rpc->auth.auth;
    
    if (rpc->ft) {
        if (rpc->auth.flavor == RPC_AUTH_UNIX) {
            vfs_set_process_uid_gid(rpc->ft->vfs, auth->uid, auth->gid);
        } else {
            vfs_set_process_uid_gid(rpc->ft->vfs, 0, 0);
        }
    }
}

int rpc_match_prog(struct rpc_t* rpc, struct rpc_prog_t* prog) {
    if (prog->prog == rpc->prog && prog->prot == rpc->prot) {
        rpc->name = prog->name;
        rpc->log  = prog->log;
        rpc->ft   = prog->ft;
        if (prog->vers == 0 || prog->vers == rpc->vers) {
            return 1;
        }
        return -1;
    }
    return 0;
}

static int rpc_call(struct csocket_t* cs, struct rpc_t* rpc) {
    int result, mismatch;
    struct rpc_prog_t* prog = rpc_prog_list;
    
    mismatch  = 0;
    rpc->low = ~0;
    rpc->high = 0;
    
    while (prog) {
        if (prog->port == cs->m_serverPort) {
            result = rpc_match_prog(rpc, prog);
            if (result > 0) {
                rpc_set_cred(rpc);
                return prog->run(rpc);
            }
            if (result < 0) {
                mismatch = 1;
                if (prog->vers < rpc->low)  rpc->low  = prog->vers;
                if (prog->vers > rpc->high) rpc->high = prog->vers;
            }
        }
        prog = prog->next;
    }
    
    return mismatch ? RPC_PROG_MISMATCH : RPC_PROG_UNAVAIL;
}

static void rpc_read_auth_unix(struct rpc_t* rpc, struct auth_unix_t* auth) {
    int i;
    
    struct xdr_t* m_in = rpc->m_in;
    int len            = rpc->auth.length;
    uint8_t* restore   = m_in->data + len;
    
    rpc->auth.auth = auth;

    if (len > m_in->size) {
        memset(rpc->auth.auth, 0, sizeof(struct auth_unix_t));
        printf("[RPC] Auth UNIX underrun\n");
        return;
    }
    
    len -= 5 * 4;
    auth->time = xdr_read_long(m_in);
    len -= xdr_read_string(m_in, auth->machine, sizeof(auth->machine));
    len &= ~3; /* align */
    auth->uid  = xdr_read_long(m_in);
    auth->gid  = xdr_read_long(m_in);
    auth->len  = xdr_read_long(m_in);
    len -= auth->len * 4;
    if (auth->len > NUM_GROUPS) auth->len = NUM_GROUPS;
    for (i = 0; i < auth->len; i++) {
        auth->gids[i] = xdr_read_long(m_in);
    }
#if DBG
    printf("RPC UNIX TIME:   %d\n", auth->time);
    printf("RPC UNIX NAME:   %s\n", auth->machine);
    printf("RPC UNIX UID:    %d\n", auth->uid);
    printf("RPC UNIX GID:    %d\n", auth->gid);
    printf("RPC UNIX LEN:    %d\n", auth->len);
    printf("RPC UNIX GIDS:   [");
    for (i = 0; i < auth->len; i++) {
        printf("%d%s", auth->gids[i], i == auth->len - 1 ? "" : ", ");
    }
    printf("]\n");
#endif
    if (len) {
        printf("[RPC] Auth UNIX decode error\n");
        m_in->data = restore;
    }
}

static void rpc_input(struct csocket_t* cs) {
    struct rpc_t rpc;
    struct auth_unix_t auth_unix;
    uint32_t status;
    uint8_t* status_ptr;
    
    struct xdr_t* m_in;
    struct xdr_t* m_out;
    
    m_in  = rpc.m_in  = cs->m_Input;
    m_out = rpc.m_out = cs->m_Output;
    
    rpc.port        = cs->m_serverPort;
    rpc.prot        = (cs->m_nType == SOCK_STREAM) ? IPPROTO_TCP : IPPROTO_UDP;
    rpc.remote_addr = cs->m_RemoteAddr.sin_addr;
    
#if DBG
    printf("RPC LEN = %d, DATA:\n", m_in->size);
    for (int i = 0; i < m_in->size; i++) {
        printf("%02x ", m_in->data[i]);
    }
    printf("\n");
#endif
    
    rpc.xid = xdr_read_long(m_in);
    rpc.msg = xdr_read_long(m_in);
    if (rpc.msg == RPC_CALL) {
        xdr_write_long(m_out, rpc.xid);
        xdr_write_long(m_out, RPC_REPLY); /* Message type */
    } else {
        printf("[RPC] %s received\n", rpc.msg == RPC_REPLY ? "Reply" : "Unknown message");
        return;
    }
    rpc.rpcvers = xdr_read_long(m_in);
    if (rpc.rpcvers == RPCVERS) {
        rpc.prog = xdr_read_long(m_in);
        rpc.vers = xdr_read_long(m_in);
        rpc.proc = xdr_read_long(m_in);
        rpc.auth.flavor = xdr_read_long(m_in);
        rpc.auth.length = xdr_read_long(m_in);
#if DBG
        printf("RPC XID:     %08x\n", rpc.xid);
        printf("RPC MSG:     %d\n",   rpc.msg);
        printf("RPC VERSION: %d\n",   rpc.rpcvers);
        printf("RPC PROG:    %d\n",   rpc.prog);
        printf("RPC PROGVER: %d\n",   rpc.vers);
        printf("RPC PROC:    %d\n",   rpc.proc);
        printf("RPC AUTH:    %d\n",   rpc.auth.flavor);
        printf("RPC AUTHLEN: %d\n",   rpc.auth.length);
#endif
        if (rpc.auth.flavor == RPC_AUTH_UNIX) {
            rpc_read_auth_unix(&rpc, &auth_unix);
        } else {
            xdr_read_skip(m_in, rpc.auth.length);
        }
        rpc.verif.flavor = xdr_read_long(m_in);
        rpc.verif.length = xdr_read_long(m_in);
        xdr_read_skip(m_in, rpc.verif.length);
#if DBG
        printf("RPC VERIF:   %d\n", rpc.verif.flavor);
        printf("RPC VERLEN:  %d\n", rpc.verif.length);
#endif
        /* RPC Reply */    
        xdr_write_long(m_out, RPC_MSG_ACCEPTED); /* Message */
        xdr_write_long(m_out, rpc.verif.flavor);
        xdr_write_long(m_out, rpc.verif.length);
        xdr_write_zero(m_out, rpc.verif.length);
        status_ptr = xdr_get_pointer(m_out);
        xdr_write_skip(m_out, 4); /* Status will be updated later */
        
        status = rpc_call(cs, &rpc);
        
        xdr_write_long_at(status_ptr, status); /* Status */
        
        if (status == RPC_PROG_MISMATCH) {
            rpc_log(&rpc, "[RPC] Version mismatch: req %d, min %d, max %d", rpc.vers, rpc.low, rpc.high);
            xdr_write_long(m_out, rpc.low);
            xdr_write_long(m_out, rpc.high);
        } else if (status == RPC_PROG_UNAVAIL) {
            printf("[RPC:%d:%d] Program not registered\n", rpc.prog, rpc.proc);
        } else if (status == RPC_GARBAGE_ARGS) {
            rpc_log(&rpc, "[RPC] Procedure cannot decode input (garbage args)");
        } else if (status == RPC_PROC_UNAVAIL) {
            rpc_log(&rpc, "[RPC] Procedure not available");
        } else if (m_in->size > 0) {
            rpc_log(&rpc, "[RPC] Unused data in buffer (%d bytes)", m_in->size);
        }
    } else { /* RPC version is not 2 */
        printf("[RPC] Version mismatch (%d)\n", rpc.rpcvers);
        xdr_write_long(m_out, RPC_MSG_DENIED); /* Message */
        xdr_write_long(m_out, RPC_MISMATCH); /* Status */
        xdr_write_long(m_out, 2); /* Min version */
        xdr_write_long(m_out, 2); /* Max version */
    }
    
#if DBG
    printf("RPC OUT = %d, DATA:\n", m_out->size);
    for (int i = 0; i < m_out->size; i++) {
        printf("%02x ", (m_out->data - m_out->size)[i]);
    }
    printf("\n");
#endif
    
    csocket_send(cs);
}

int proc_null(struct rpc_t* rpc) {
    rpc_log(rpc, "NULL");
    return RPC_SUCCESS;
}


static void print_about(void) {
    static int show = 1;
    
    if (show) {
        printf("[NFSD] Network File System server\n");
        printf("[NFSD] Copyright (C) 2005 Ming-Yang Kao\n");
        printf("[NFSD] Edited in 2011 by ZeWaren\n");
        printf("[NFSD] Edited in 2013 by Alexander Schneider (Jankowfsky AG)\n");
        printf("[NFSD] Edited in 2014 2015 by Yann Schepens\n");
        printf("[NFSD] Edited in 2016 by Peter Philipp (Cando Image GmbH), Marc Harding\n");
        printf("[NFSD] Mostly rewritten in 2019-2021 by Simon Schubiger for Previous NeXT emulator\n");
        printf("[NFSD] Rewritten in C in 2025 by Andreas Grabher\n");
    }
    show = 0;
}

void rpc_add_program(struct rpc_prog_t* prog) {
    struct rpc_prog_t** entry = &rpc_prog_list;
    
    while (*entry) {
        entry = &(*entry)->next;
    }
    
    *entry = prog;
    (*entry)->next = NULL;
    
    if (prog->prot == IPPROTO_TCP) {
        prog->sock = tcpsocket_init(rpc_input);
        if (prog->sock) {
            prog->port = tcpsocket_open(prog->sock, prog->port);
            if (prog->port) {
                printf("[RPC] %s daemon started (TCP: %d -> %d).\n", prog->name, prog->port, 
                       tcpsocket_toLocalPort(prog->port));
            } else {
                printf("[RPC] %s daemon start failed.\n", prog->name);
                tcpsocket_close(prog->sock);
                prog->sock = tcpsocket_uninit(prog->sock);
            }
        } else {
            printf("[RPC] Socket initialisation failed.");
        }
    } else {
        prog->sock = udpsocket_init(rpc_input);
        if (prog->sock) {
            prog->port = udpsocket_open(prog->sock, prog->port);
            if (prog->port) {
                printf("[RPC] %s daemon started (UDP: %d -> %d).\n", prog->name, prog->port, 
                       udpsocket_toLocalPort(prog->port));
            } else {
                printf("[RPC] %s daemon start failed.\n", prog->name);
                udpsocket_close(prog->sock);
                prog->sock = udpsocket_uninit(prog->sock);
            }
        } else {
            printf("[RPC] Socket initialisation failed.");
        }
    }
}

static void rpc_remove_all_programs(void) {
    struct rpc_prog_t** entry = &rpc_prog_list;
    struct rpc_prog_t* next;
    
    while (*entry) {
        if ((*entry)->sock) {
            if ((*entry)->prot == IPPROTO_TCP) {
                (*entry)->sock = tcpsocket_uninit((*entry)->sock);
            } else {
                (*entry)->sock = udpsocket_uninit((*entry)->sock);
            }
        }
        next = (*entry)->next;
        free(*entry);
        *entry = next;
    }
}

static int inited = 0;

void rpc_reset(void) {
    if (access(ConfigureParams.Ethernet.szNFSroot, F_OK | R_OK | W_OK) < 0) {
        printf("[RPC] can not access directory '%s'. NFS startup canceled.\n", ConfigureParams.Ethernet.szNFSroot);
        rpc_uninit();
        nfsd_fts[0] = ft_uninit(nfsd_fts[0]);
        return;
    }
    
    if (ft_is_inited(nfsd_fts[0])) {
        if (ft_path_changed(nfsd_fts[0], ConfigureParams.Ethernet.szNFSroot)) {
            rpc_uninit();
            nfsd_fts[0] = ft_init(ConfigureParams.Ethernet.szNFSroot, "/");
        }
    } else {
        nfsd_fts[0] = ft_init(ConfigureParams.Ethernet.szNFSroot, "/");
    }
    if (nfsd_fts[0]) {
        rpc_init(nfsd_fts[0]);
    }
}

void rpc_init(struct ft_t* ft) {
    int i;
    struct rpc_prog_t* prog;
    char hostname[NAME_HOST_MAX];
    
    if (inited) return;
    
    memset(hostname, 0, sizeof(hostname));
    gethostname(hostname, sizeof(hostname));
    
    print_about();
    printf("[RPC] starting local NFS daemon on '%s', exporting '%s'\n", hostname, ConfigureParams.Ethernet.szNFSroot);
    
    vdns_uninit();
    rpc_remove_all_programs();
    
    for (i = 0; i < TBL_SIZE(rpc_prog_table_template); i++) {
        prog = (struct rpc_prog_t*)malloc(sizeof(struct rpc_prog_t));
        *prog = rpc_prog_table_template[i];
        prog->ft = ft;
        rpc_add_program(prog);
    }
    
    nibind_init();
    vdns_init();
    
    inited = 1;
}

void rpc_uninit(void) {
    if (inited) {
        rpc_remove_all_programs();
        
        nfsd_fts[0] = ft_uninit(nfsd_fts[0]);
        nibind_uninit();
        vdns_uninit();
        mount_uninit();
        
        inited = 0;
    }
}

int rpc_read_file(const char* vfs_path, size_t offset, uint8_t* data, size_t len) {
    if (inited) {
        struct path_t path;
        vfscpy(path.vfs, vfs_path, sizeof(path.vfs));
        vfs_to_host_path(nfsd_fts[0]->vfs, &path);
        return vfs_read(&path, offset, data, len);
    }
    return -1;
}

int rpc_match_addr(uint32_t addr) {
    if ((addr == (ntohl(special_addr.s_addr) | CTL_NFSD)) ||
        (addr == (ntohl(special_addr.s_addr) | ~(uint32_t)CTL_NET_MASK))) {
        return 1;
    }
    /* NS kernel to broadcasts on 10.255.255.255 */
    if (addr == (ntohl(special_addr.s_addr) | ~(uint32_t)CTL_CLASS_MASK(CTL_NET))) {
        return 1;
    }
    return 0;
}

void rpc_udp_map_to_local_port(struct in_addr* ipNBO, uint16_t* dportNBO) {
    uint16_t dport = ntohs(*dportNBO);
    uint16_t port  = udpsocket_toLocalPort(dport);
    if(port) {
        *dportNBO = htons(port);
        *ipNBO    = loopback_addr;
    }
}

void rpc_tcp_map_to_local_port(uint16_t port, uint16_t* sin_portNBO) {
    uint16_t localPort = tcpsocket_toLocalPort(port);
    if(localPort)
        *sin_portNBO = htons(localPort);
}

void rpc_udp_map_from_local_port(uint16_t port, struct in_addr* saddrNBO, uint16_t* sin_portNBO) {
    uint16_t localPort = udpsocket_fromLocalPort(port);
    if(localPort) {
        *sin_portNBO = htons(localPort);
        switch(localPort) {
            case PORT_DNS:
                saddrNBO->s_addr = special_addr.s_addr | htonl(CTL_DNS);
                break;
            default:
                saddrNBO->s_addr = special_addr.s_addr | htonl(CTL_NFSD);
                break;
        }
    }
}

void rpc_log(struct rpc_t* rpc, const char *format, ...) {
    va_list vargs;
    
    if (rpc->log)
    {
        va_start(vargs, format);
        printf("[RPC:%s:%d] ", rpc->name, rpc->proc);
        vprintf(format, vargs);
        printf("\n");
        va_end(vargs);
    }
}
