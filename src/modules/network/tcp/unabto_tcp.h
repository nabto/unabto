#ifndef _UNABTO_TCP_H_
#define _UNABTO_TCP_H_
#include <unabto/unabto_context.h>

#ifdef WIN32
#include <basetsd.h>
#include "windows/unabto_tcp_windows.h"
#else
#include "unix/unabto_tcp_unix.h"
#include <unistd.h>
#endif

typedef enum {
    UTS_OK,
    UTS_WOULD_BLOCK,
    UTS_FAILED,
    UTS_CONNECTING,
    UTS_EOF // no more data from the other side
} unabto_tcp_status;

/**
 * Write data to a tcp connection.
 *
 * @return
 *  UTS_OK if some bytes were written.
 *  UTS_WOULD_BLOCK if no bytes could be written.
 *  UTS_FAILED if the stream has been closed or some other error occured.
 */
unabto_tcp_status unabto_tcp_write(struct unabto_tcp_socket* sock, const void* buf, const size_t len, size_t* written);

/**
 * Read data from a tcp connection.
 *
 * @return
 *  UTS_OK iff some bytes were read.
 *  UTS_WOULD_BLOCK iff no bytes were ready.
 *  UTS_EOF iff eof has been reached.
 *  UTS_FAILED if some other error occurred.
 */
unabto_tcp_status unabto_tcp_read(struct unabto_tcp_socket* sock, void* buf, const size_t len, size_t* written);

unabto_tcp_status unabto_tcp_close(struct unabto_tcp_socket* sock);

unabto_tcp_status unabto_tcp_shutdown(struct unabto_tcp_socket* sock);

/**
 * Open a tcp socket. epollDataPtr is a pointer to some data such that
 * it's possible to find who should handle the epoll event.
 */
unabto_tcp_status unabto_tcp_open(struct unabto_tcp_socket* sockfd, enum nabto_ip_address_type addressType, void* epollDataPtr);

unabto_tcp_status unabto_tcp_connect(struct unabto_tcp_socket* sock, nabto_endpoint* ep);

/* Polls if socket has been connected
 */
unabto_tcp_status unabto_tcp_connect_poll(struct unabto_tcp_socket* sock);


#endif //_UNABTO_TCP_H_
