#ifndef _UNABTO_TCP_UNIX_H_
#define _UNABTO_TCP_UNIX_H_
#include <netinet/in.h>

#define INVALID_SOCKET (-1)

struct unabto_tcp_socket {
    int socket;
};

#endif //_UNABTO_TCP_UNIX_H_
