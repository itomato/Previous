/*
 * Mount Program
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
#include "mount.h"


enum {
    MNT_OK        = 0,
    MNTERR_PERM   = 1,
    MNTERR_NOENT  = 2,
    MNTERR_IO     = 5,
    MNTERR_ACCESS = 13,
    MNTERR_NOTDIR = 20,
    MNTERR_INVAL  = 22
};

struct mount_t {
    char* name;
    char* path;
    struct mount_t* next;
};

static struct mount_t* rmtab = NULL;

static int mnt_add(char* name, char* path) {
    struct mount_t** entry = &rmtab;
    
    while (*entry) {
        if (strncmp((*entry)->name, name, INET_ADDRSTRLEN) || 
            strncmp((*entry)->path, path, MAXPATHLEN)) {
            entry = &(*entry)->next;
            continue;
        }
        return 1;
    }
    *entry = (struct mount_t*)malloc(sizeof(struct mount_t));
    (*entry)->name = strdup(name);
    (*entry)->path = strdup(path);
    (*entry)->next = NULL;
    return 0;
}

static int mnt_remove(char* name, char* path) {
    struct mount_t** entry = &rmtab;
    struct mount_t* next = NULL;
    
    while (*entry) {
        if (strncmp((*entry)->name, name, INET_ADDRSTRLEN) || 
            strncmp((*entry)->path, path, MAXPATHLEN)) {
            entry = &(*entry)->next;
            continue;
        }
        free((*entry)->name);
        free((*entry)->path);
        next = (*entry)->next;
        free((*entry));
        *entry = next;
        return 1;
    }
    return 0;
}

static void mnt_delete(void) {
    struct mount_t** entry = &rmtab;
    struct mount_t* next = NULL;

    while (*entry) {
        free((*entry)->name);
        free((*entry)->path);
        next = (*entry)->next;
        free((*entry));
        *entry = next;
    }
}


static int proc_mnt(struct rpc_t* rpc) {
    struct path_t path;
    char name[INET_ADDRSTRLEN];
    uint64_t handle;
    
    inet_ntop(AF_INET, &rpc->remote_addr, name, INET_ADDRSTRLEN);
    
    struct xdr_t* m_in  = rpc->m_in;
    struct xdr_t* m_out = rpc->m_out;
    
    if (xdr_read_string(m_in, path.vfs, sizeof(path.vfs)) < 0) return RPC_GARBAGE_ARGS;
    vfs_to_host_path(rpc->ft->vfs, &path);
    
    rpc_log(rpc, "MNT from %s for '%s'", name, path.vfs);
    
    handle = ft_get_fhandle(rpc->ft, &path);
    if (handle) {
        uint64_t data[8] = {handle, 0, 0, 0, 0, 0, 0, 0};
        
        xdr_write_long(m_out, MNT_OK);
        
        if (rpc->vers == 1 || rpc->vers == 2) {
            xdr_write_data(m_out, data, FHSIZE);
        } else {
            xdr_write_long(m_out, FHSIZE_NFS3);
            xdr_write_data(m_out, data, FHSIZE_NFS3);
            xdr_write_long(m_out, 0);  /* flavor */
        }
        
        if (mnt_add(name, path.vfs)) {
            rpc_log(rpc, "MNT '%s' already mounted from %s", path, name);
        }
    } else {
        xdr_write_long(m_out, MNTERR_ACCESS);  /* Permission denied */
    }
    
    return RPC_SUCCESS;
}

static int proc_umnt(struct rpc_t* rpc) {
    char path[MAXPATHLEN];
    char name[INET_ADDRSTRLEN];
    
    inet_ntop(AF_INET, &rpc->remote_addr, name, INET_ADDRSTRLEN);
    
    int found = 0;
    
    struct xdr_t* m_in  = rpc->m_in;
    struct xdr_t* m_out = rpc->m_out;

    if (xdr_read_string(m_in, path, sizeof(path)) < 0) return RPC_GARBAGE_ARGS;
    
    rpc_log(rpc, "UNMT from %s for '%s'", name, path);
    
    found = mnt_remove(name, path);
    
    if (!found) {
        rpc_log(rpc, "UMNT '%s' not mounted from %s", path, name);
    }
    
    xdr_write_long(m_out, found ? MNT_OK : MNTERR_NOTDIR);
    
    return RPC_SUCCESS;
}

static int proc_export(struct rpc_t* rpc) {
    char path[MAXPATHLEN];
    uint8_t group[4] = { '*', '.', '.', '.' }; /* "*..." */
    
    struct xdr_t* m_out = rpc->m_out;
    
    rpc_log(rpc, "EXPORT");
    
    vfs_get_basepath_alias(rpc->ft->vfs, path, sizeof(path));
    
    /* dirpath */
    xdr_write_long(m_out, 1);
    xdr_write_string(m_out, path, sizeof(path));

    /* groups */
    xdr_write_long(m_out, 1);
    xdr_write_long(m_out, 1);
    xdr_write_data(m_out, group, 4);
    xdr_write_long(m_out, 0);
    
    xdr_write_long(m_out, 0);
    xdr_write_long(m_out, 0);
    
    return RPC_SUCCESS;
}


int mount_prog(struct rpc_t* rpc) {
    switch (rpc->proc) {
        case MOUNTPROC_NULL:
            return proc_null(rpc);
            
        case MOUNTPROC_MNT:
            return proc_mnt(rpc);
            
        case MOUNTPROC_DUMP:
            rpc_log(rpc, "DUMP unimplemented");
            return RPC_PROC_UNAVAIL;
            
        case MOUNTPROC_UMNT:
            return proc_umnt(rpc);
            
        case MOUNTPROC_UMNTALL:
            rpc_log(rpc, "UMNTALL unimplemented");
            return RPC_PROC_UNAVAIL;
            
        case MOUNTPROC_EXPORT:
            return proc_export(rpc);
            
        case MOUNTPROC_EXPORTALL:
            rpc_log(rpc, "EXPORTALL unimplemented");
            return RPC_PROC_UNAVAIL;

        default:
            break;
    }
    rpc_log(rpc, "Process unavailable");
    return RPC_PROC_UNAVAIL;
}

void mount_uninit(void) {
    mnt_delete();
}
