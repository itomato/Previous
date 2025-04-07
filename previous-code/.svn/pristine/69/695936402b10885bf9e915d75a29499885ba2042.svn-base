/*
 * NetInfo Bind Program
 * 
 * Created by Simon Schubiger on 09.01.2021
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
#include "netinfobind.h"


struct ni_prog_t* ni_register;

const struct rpc_prog_t ni_rpc_prog_template = 
{
    NETINFOPROG, NETINFOVERS, 0, 0, netinfo_prog, 1, "NETINFO", NULL, NULL, NULL
};

const struct ni_prog_t ni_prog_table_template[] = 
{
    { "local",   NULL, NULL, 0,            0,            NULL, NULL, NULL },
    { "network", NULL, NULL, PORT_NETINFO, PORT_NETINFO, NULL, NULL, NULL }
};

#if 0
static void ni_register_program(struct ni_prog_t* prog) {
    prog->udp_port = prog->udp_prog->port;
    prog->tcp_port = prog->tcp_prog->port;
    printf("[NETINFOBIND] Registering '%s' at udp:%d, tcp:%d\n", prog->tag, prog->udp_port, prog->tcp_port);
}
#endif

static void ni_add_program(struct ni_prog_t* prog) {
    struct ni_prog_t** entry = &ni_register;
        
    prog->udp_prog = (struct rpc_prog_t*)malloc(sizeof(struct rpc_prog_t));
    prog->tcp_prog = (struct rpc_prog_t*)malloc(sizeof(struct rpc_prog_t));
    
    *(prog->udp_prog) = ni_rpc_prog_template;
    prog->udp_prog->prot = IPPROTO_UDP;
    prog->udp_prog->port = prog->udp_port;
    
    *(prog->tcp_prog) = ni_rpc_prog_template;
    prog->tcp_prog->prot = IPPROTO_TCP;
    prog->tcp_prog->port = prog->tcp_port;

    rpc_add_program(prog->udp_prog);
    rpc_add_program(prog->tcp_prog);
    
    prog->udp_port = prog->udp_prog->port;
    prog->tcp_port = prog->tcp_prog->port;
    
    printf("[NETINFOBIND] Registering '%s' at udp:%d, tcp:%d\n", prog->tag, prog->udp_port, prog->tcp_port);

    while (*entry) {
        entry = &(*entry)->next;
    }
    
    *entry = prog;
    (*entry)->next = NULL;
}

void nibind_init(void) {
    int i;
    
    struct ni_prog_t* prog;
    for (i = 0; i < TBL_SIZE(ni_prog_table_template); i++) {
        prog = (struct ni_prog_t*)malloc(sizeof(struct ni_prog_t));
        *prog = ni_prog_table_template[i];
        ni_add_program(prog);
    }
    
    netinfo_build_nidb();
}

void nibind_uninit(void) {
    struct ni_prog_t* next;
    
    netinfo_delete_nidb();
    
    while (ni_register) {
        next = ni_register->next;
        free(ni_register);
        ni_register = next;
    }
}


static int proc_register(struct rpc_t* rpc) {
    rpc_log(rpc, "REGISTER unimplemented");

    return RPC_PROC_UNAVAIL;
}
static int proc_unregister(struct rpc_t* rpc) {
    rpc_log(rpc, "UNREGISTER unimplemented");

    return RPC_PROC_UNAVAIL;
}

static int proc_getregister(struct rpc_t* rpc) {
    struct ni_prog_t* prog = NULL;
    char tag[MAXNAMELEN+1];
    
    struct xdr_t* m_in  = rpc->m_in;
    struct xdr_t* m_out = rpc->m_out;
    
    if (xdr_read_string(m_in, tag, sizeof(tag)) < 0) return RPC_GARBAGE_ARGS;
    
    prog = ni_register;
    while (prog) {
        if (strncmp(prog->tag, tag, MAXNAMELEN) == 0) {
            break;
        }
        prog = prog->next;
    }
    
    if (prog == NULL) {
        rpc_log(rpc, "GETREGISTER no tag '%s'", tag);
        xdr_write_long(m_out, NI_NOTAG);
    } else {
        rpc_log(rpc, "GETREGISTER '%s' at udp:%d, tcp:%d", tag, prog->udp_port, prog->tcp_port);
        xdr_write_long(m_out, NI_OK);
        xdr_write_long(m_out, prog->udp_port);
        xdr_write_long(m_out, prog->tcp_port);
    }
    
    return RPC_SUCCESS;
}

static int proc_listreg(struct rpc_t* rpc) {
    rpc_log(rpc, "LISTREG unimplemented");
    
    return RPC_PROC_UNAVAIL;
}

static int proc_createmaster(struct rpc_t* rpc) {
    rpc_log(rpc, "CREATEMASTER unimplemented");
    
    return RPC_PROC_UNAVAIL;
}

static int proc_createclone(struct rpc_t* rpc) {
    rpc_log(rpc, "CREATECLONE unimplemented");
    
    return RPC_PROC_UNAVAIL;
}

static int proc_destroydomain(struct rpc_t* rpc) {
    rpc_log(rpc, "DESTROYDOMAIN unimplemented");
    
    return RPC_PROC_UNAVAIL;
}

static int proc_bind(struct rpc_t* rpc) {
    uint32_t clientAddr;
    char clientTag[MAXNAMELEN+1];
    char serverTag[MAXNAMELEN+1];

    struct xdr_t* m_in  = rpc->m_in;
    struct xdr_t* m_out = rpc->m_out;
    
    if (m_in->size < 4) return RPC_GARBAGE_ARGS;
    clientAddr = xdr_read_long(m_in);
    
    if (xdr_read_string(m_in, clientTag, sizeof(clientTag)) < 0) return RPC_GARBAGE_ARGS;
    if (xdr_read_string(m_in, serverTag, sizeof(serverTag)) < 0) return RPC_GARBAGE_ARGS;
    
    rpc_log(rpc, "BIND '%s' to '%s'", clientTag, serverTag);

    xdr_write_long(m_out, NI_OK);
    
    return RPC_SUCCESS;
}


int nibind_prog(struct rpc_t* rpc) {
    switch (rpc->proc) {
        case NIBINDPROC_NULL:
            return proc_null(rpc);
            
        case NIBINDPROC_REGISTER:
            return proc_register(rpc);
            
        case NIBINDPROC_UNREGISTER:
            return proc_unregister(rpc);

        case NIBINDPROC_GETREGISTER:
            return proc_getregister(rpc);

        case NIBINDPROC_LISTREG:
            return proc_listreg(rpc);

        case NIBINDPROC_CREATEMASTER:
            return proc_createmaster(rpc);

        case NIBINDPROC_CREATECLONE:
            return proc_createclone(rpc);

        case NIBINDPROC_DESTROYDOMAIN:
            return proc_destroydomain(rpc);

        case NIBINDPROC_BIND:
            return proc_bind(rpc);

        default:
            break;
    }
    rpc_log(rpc, "Process unavailable");
    return RPC_PROC_UNAVAIL;
}
