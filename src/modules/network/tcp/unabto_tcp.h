#ifndef _UNABTO_TCP_H_
#define _UNABTO_TCP_H_
#include <unistd.h>
#include <unabto/unabto_context.h>

#ifdef WIN32
#include "windows/unabto_tcp_windows.h"
#else
#include "unix/unabto_tcp_unix.h"
#endif

enum unabto_tcp_status {
    UTS_OK,
    UTS_WOULD_BLOCK,
    UTS_FAILED,
    UTS_CONNECTING
};


enum unabto_tcp_status unabto_tcp_write(struct unabto_tcp_socket* sock, const void* buf, const size_t len, size_t* written);

enum unabto_tcp_status unabto_tcp_read(struct unabto_tcp_socket* sock, void* buf, const size_t len, size_t* written);

enum unabto_tcp_status unabto_tcp_close(struct unabto_tcp_socket* sock);

enum unabto_tcp_status unabto_tcp_shutdown(struct unabto_tcp_socket* sock);

enum unabto_tcp_status unabto_tcp_open(struct unabto_tcp_socket* sockfd);

enum unabto_tcp_status unabto_tcp_connect(struct unabto_tcp_socket* sock, nabto_endpoint ep);

/* Polls if socket has been connected
 */
enum unabto_tcp_status unabto_tcp_connect_poll(struct unabto_tcp_socket* sock);


#endif //_UNABTO_TCP_H_
