#include <modules/network/tcp/unabto_tcp.h>
#include <platforms/unabto_common_types.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/tcp.h>

#if NABTO_ENABLE_EPOLL
#include <modules/network/epoll/unabto_epoll.h>
#include <sys/epoll.h>
#endif

#ifdef __MACH__
#define MSG_NOSIGNAL 0
#endif

unabto_tcp_status unabto_tcp_read(struct unabto_tcp_socket* sock, void* buf, const size_t len, size_t* read) {
    int status;
    int err;
    
    status = recv(sock->socket, buf, len, 0);
    err = errno;
    if (status < 0) {
        if ((err == EAGAIN) || err == EWOULDBLOCK) {
            return UTS_WOULD_BLOCK;
        } else {
            NABTO_LOG_ERROR(("unabto_tcp_read failed"));
            return UTS_FAILED;
        }
    } else if (status == 0) {
        NABTO_LOG_TRACE(("TCP connection closed by peer"));
        return UTS_FAILED;
    } else {
        *read = status;
        return UTS_OK;
    }
}

unabto_tcp_status unabto_tcp_write(struct unabto_tcp_socket* sock, const void* buf, const size_t len, size_t* written){
    int status;
    NABTO_LOG_TRACE(("Writing %i bytes to tcp socket", len));
    status = send(sock->socket, buf, len, MSG_NOSIGNAL);
    NABTO_LOG_TRACE(("tcp send status: %i", status));
    if (status > 0) {
        *written = status;
        return UTS_OK;
    } else if (status < 0) {
        int err = errno;
        if ((err == EAGAIN) || err == EWOULDBLOCK) {
            return UTS_WOULD_BLOCK;
        } else {
            NABTO_LOG_ERROR(("Send of tcp packet failed"));
            return UTS_FAILED; 
        }
    }
    return UTS_OK;
}

unabto_tcp_status unabto_tcp_close(struct unabto_tcp_socket* sock){
    if (sock->socket == INVALID_SOCKET) {
        NABTO_LOG_ERROR(("trying to close invalid socket"));
    } else {
#if NABTO_ENABLE_EPOLL
        {
            struct epoll_event ev;
            memset(&ev, 0, sizeof(struct epoll_event));
            epoll_ctl(unabto_epoll_fd, EPOLL_CTL_DEL, sock->socket, &ev);
        }
#endif
        close(sock->socket);
        sock->socket = INVALID_SOCKET;
    }
    return UTS_OK;
}

unabto_tcp_status unabto_tcp_shutdown(struct unabto_tcp_socket* sock){
    shutdown(sock->socket, SHUT_WR);
    return UTS_OK;
}

unabto_tcp_status unabto_tcp_open(struct unabto_tcp_socket* sock, void* epollDataPtr){
    sock->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (sock->socket < 0) {
        NABTO_LOG_ERROR(("Could not create socket for TCP"));
        return UTS_FAILED;
    }
#if NABTO_ENABLE_EPOLL
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
    ev.data.ptr = epollDataPtr;
    epoll_ctl(unabto_epoll_fd, EPOLL_CTL_ADD, sock->socket, &ev);
#endif

    {
        int flags = 1;
        if (setsockopt(sock->socket, IPPROTO_TCP, TCP_NODELAY, (char *) &flags, sizeof(int)) != 0) {
            NABTO_LOG_ERROR(("Could not set socket option TCP_NODELAY"));
        }
        flags = fcntl(sock->socket, F_GETFL, 0);
        if (flags < 0) {
            NABTO_LOG_ERROR(("fcntl failed in F_GETFL"));
            unabto_tcp_close(sock);
            return UTS_FAILED;
        }
        if (fcntl(sock->socket, F_SETFL, flags | O_NONBLOCK) < 0) {
            NABTO_LOG_ERROR(("fcntl failed in F_SETFL"));
            unabto_tcp_close(sock);
            return UTS_FAILED;
        }

        flags = 1;
        if(setsockopt(sock->socket, SOL_SOCKET, SO_KEEPALIVE, &flags, sizeof(flags)) < 0) {
            NABTO_LOG_ERROR(("could not enable KEEPALIVE"));
        }

#ifndef __MACH__
        flags = 9;
        if(setsockopt(sock->socket, SOL_TCP, TCP_KEEPCNT, &flags, sizeof(flags)) < 0) {
            NABTO_LOG_ERROR(("could not set TCP_KEEPCNT"));
        }

        flags = 60;
        if(setsockopt(sock->socket, SOL_TCP, TCP_KEEPIDLE, &flags, sizeof(flags)) < 0) {
            NABTO_LOG_ERROR(("could not set TCP_KEEPIDLE"));
        }

        flags = 60;
        if(setsockopt(sock->socket, SOL_TCP, TCP_KEEPINTVL, &flags, sizeof(flags)) < 0) {
            NABTO_LOG_ERROR(("could not set TCP KEEPINTVL"));
        }
#else
        flags = 60;
        if(setsockopt(sock->socket, IPPROTO_TCP, TCP_KEEPALIVE, &flags, sizeof(flags)) < 0) {
            NABTO_LOG_ERROR(("could not set TCP_KEEPCNT"));
        }
#endif
        return UTS_OK;
    }
}


unabto_tcp_status unabto_tcp_connect(struct unabto_tcp_socket* sock, nabto_endpoint* ep){
    int status;
    struct sockaddr_in host;

    memset(&host,0,sizeof(struct sockaddr_in));
    host.sin_family = AF_INET;
    host.sin_addr.s_addr = htonl(ep->addr);
    host.sin_port = htons(ep->port);
    NABTO_LOG_TRACE(("Connecting to ", PRIep, MAKE_EP_PRINTABLE(*ep)));
    
    status = connect(sock->socket, (struct sockaddr*)&host, sizeof(struct sockaddr_in));
   
    if (status == 0) {
        return UTS_OK;
    } else {
        int err = errno;
        if (err == EINPROGRESS) {
            return UTS_CONNECTING;
        } else {
            NABTO_LOG_ERROR(("Could not connect to tcp endpoint. %s", strerror(errno)));
            return UTS_FAILED;
        }
    }
}


/* Polls if socket has been connected
 */
unabto_tcp_status unabto_tcp_connect_poll(struct unabto_tcp_socket* sock){
    int err;
    socklen_t len;
    len = sizeof(err);
    if (getsockopt(sock->socket, SOL_SOCKET, SO_ERROR, &err, &len) != 0) {
        return UTS_FAILED;
    } else {
        if (err == 0) {
            return UTS_OK;
        } else if ( err == EINPROGRESS) {
            return UTS_CONNECTING;
        } else {
            NABTO_LOG_ERROR(("Socket not open %d", err));
            return UTS_FAILED;
        }
    }
    return UTS_OK;
}

