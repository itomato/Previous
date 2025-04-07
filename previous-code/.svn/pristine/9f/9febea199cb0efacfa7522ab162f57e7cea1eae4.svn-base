/* UDP Server Socket */

#ifndef _UDPSOCKET_H_
#define _UDPSOCKET_H_

#include "csocket.h"

struct udpsocket_t {
    uint16_t           m_nPort;
    sock_t             m_Socket;
    struct csocket_t*  m_pSocket;
    int                m_nClosed;
    socket_listener_t* m_pListener;
};

struct udpsocket_t* udpsocket_init(socket_listener_t* pListener);
struct udpsocket_t* udpsocket_uninit(struct udpsocket_t* us);

uint16_t udpsocket_open(struct udpsocket_t* us, uint16_t nPort);
void udpsocket_close(struct udpsocket_t* us);
int  udpsocket_getPort(struct udpsocket_t* us);
void udpspcket_run(struct udpsocket_t* us);

void      udpsocket_portMap(uint16_t src, uint16_t local);
void      udpsocket_portUnmap(uint16_t src);
uint16_t  udpsocket_toLocalPort(uint16_t src);
uint16_t  udpsocket_fromLocalPort(uint16_t local);

#endif /* _UDPSOCKET_H_ */
