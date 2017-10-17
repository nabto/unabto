#ifndef _UNABTO_TCP_UNIX_H_
#define _UNABTO_TCP_UNIX_H_
#include <netinet/in.h>

struct unabto_tcp_socket {
    int socket;
    struct sockaddr_in host;
};

#endif //_UNABTO_TCP_UNIX_H_
