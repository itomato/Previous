/* TCP Server Socket */

#ifndef _TCPSOCKET_H_
#define _TCPSOCKET_H_

#include "csocket.h"

struct tcpsocket_t {
    uint16_t           m_nPort;
    sock_t             m_ServerSocket;
    struct csocket_t** m_pSockets;
    int                m_nClosed;
    socket_listener_t* m_pListener;
    thread_t*          m_hThread;
};

struct tcpsocket_t* tcpsocket_init(socket_listener_t* pListener);
struct tcpsocket_t* tcpsocket_uninit(struct tcpsocket_t* ts);

uint16_t tcpsocket_open(struct tcpsocket_t* ts, uint16_t nPort);
void tcpsocket_close(struct tcpsocket_t* ts);
int  tcpsocket_getPort(struct tcpsocket_t* ts);
void tcpsocket_run(struct tcpsocket_t* ts);

void      tcpsocket_portMap(uint16_t src, uint16_t local);
void      tcpsocket_portUnmap(uint16_t src);
uint16_t  tcpsocket_toLocalPort(uint16_t src);
uint16_t  tcpsocket_fromLocalPort(uint16_t local);

#endif /* _TCPSOCKET_H_ */
