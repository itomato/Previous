/* Remote Procecure Call  */

#ifndef _RPC_H_
#define _RPC_H_

#include "xdr.h"
#include "filetable.h"

#define PORT_RPC 111

#define RPCVERS 2

/* Message */
#define RPC_CALL          0
#define RPC_REPLY         1

/* Reject status */
#define RPC_MISMATCH      0  /* RPC version number != 2          */
#define RPC_AUTH_ERROR    1  /* remote can't authenticate caller */

/* Reply status */
#define RPC_MSG_ACCEPTED  0
#define RPC_MSG_DENIED    1

/* Accepted status */
#define RPC_SUCCESS       0
#define RPC_PROG_UNAVAIL  1
#define RPC_PROG_MISMATCH 2
#define RPC_PROC_UNAVAIL  3
#define RPC_GARBAGE_ARGS  4

/* Authentication type */
#define RPC_AUTH_NONE     0
#define RPC_AUTH_UNIX     1
#define RPC_AUTH_SHORT    2
#define RPC_AUTH_DES      3

/* Authentication constants */
#define MAX_MACHINE_NAME 255
#define NUM_GROUPS       16

/* The size in bytes of the opaque file handle. */
#define FHSIZE      32
#define FHSIZE_NFS3 64


struct auth_unix_t {
    uint32_t time;
    char machine[MAX_MACHINE_NAME];
    uint32_t uid;
    uint32_t gid;
    uint32_t len;
    uint32_t gids[NUM_GROUPS];
};

struct cred_t {
    uint32_t flavor;
    uint32_t length;
    void* auth;
};

struct rpc_t {
    uint32_t xid;
    uint32_t msg;
    uint32_t rpcvers;
    uint32_t prog;
    uint32_t vers;
    uint32_t proc;
    struct cred_t auth;
    struct cred_t verif;
    
    struct xdr_t* m_in;
    struct xdr_t* m_out;
    
    struct ft_t* ft;
    
    uint16_t port;
    uint32_t prot;
    uint32_t low;
    uint32_t high;
    
    struct in_addr remote_addr;
    
    int log;
    const char* name;
};

struct rpc_prog_t {
    uint32_t prog;
    uint32_t vers;
    uint16_t prot;
    uint16_t port;
    
    int (*run)(struct rpc_t* rpc);
    int          log;
    const char*  name;
    void*        sock;
    struct ft_t* ft;
    
    struct rpc_prog_t* next;
};

extern struct rpc_prog_t* rpc_prog_list;

#define TBL_SIZE(x) (sizeof(x)/sizeof(x[0]))

void rpc_add_program(struct rpc_prog_t* prog);

int proc_null(struct rpc_t* rpc);

int rpc_match_prog(struct rpc_t* rpc, struct rpc_prog_t* prog);

void rpc_reset(void);
void rpc_init(struct ft_t* ft);
void rpc_uninit(void);

void rpc_log(struct rpc_t* rpc, const char *format, ...);

int rpc_read_file(const char* vfs_path, size_t offset, uint8_t* data, size_t len);

int rpc_match_addr(uint32_t addr);
void rpc_udp_map_to_local_port(struct in_addr* ipNBO, uint16_t* dportNBO);
void rpc_tcp_map_to_local_port(uint16_t port, uint16_t* sin_portNBO);
void rpc_udp_map_from_local_port(uint16_t port, struct in_addr* saddrNBO, uint16_t* sin_portNBO);

#endif /* _RPC_H_ */
