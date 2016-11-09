 
#include <unabto/unabto_stream.h>
#include <unabto/unabto_memory.h>
#include <errno.h>
#include <fcntl.h>
#include <modules/network/epoll/unabto_epoll.h>

#include <modules/tunnel_common/tunnel_common.h>
#include <modules/tunnel_common/tunnel_tcp.h>

#if NABTO_ENABLE_EPOLL
#include <sys/epoll.h>
#endif

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


extern void close_stream_reader(tunnel* tunnel);


/**
 * read from tcp, write to unabto
 */
 
 
void tcp_forward(tunnel* tunnel) {
    while(true) {
        unabto_stream_hint hint;
        size_t canWriteToStreamBytes;
        
        if (tunnel->extReadState == FS_CLOSING) {
            break;
        }

        canWriteToStreamBytes = unabto_stream_can_write(tunnel->stream, &hint);
        if (hint != UNABTO_STREAM_HINT_OK) {
            close_reader(tunnel);
            break;
        }
        if (canWriteToStreamBytes == 0) {
            tunnel->extReadState = FS_WRITE;
            break;
        } else {
            tunnel->extReadState = FS_READ;
        }
        {
            ssize_t readen;
            uint8_t readBuffer[NABTO_MEMORY_STREAM_SEND_SEGMENT_SIZE];
            size_t maxRead = MIN(NABTO_MEMORY_STREAM_SEND_SEGMENT_SIZE, canWriteToStreamBytes);
            
            readen = recv(tunnel->tunnel_type_vars.tcp.sock, readBuffer, maxRead, 0);
            if (readen == 0) {
                // eof
                close_reader(tunnel);
                break;
            } else if (readen < 0) {
#if defined(WINSOCK)
                if (WSAGetLastError() == WSAEWOULDBLOCK) {
                    break;
#else
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
#endif
                } else {
                    close_reader(tunnel);
                    break;
                }
            } else if (readen > 0) {
            
                unabto_stream_hint hint;
                size_t written = unabto_stream_write(tunnel->stream, readBuffer, readen, &hint);
                if (hint != UNABTO_STREAM_HINT_OK) {
                    NABTO_LOG_TRACE(("Can't write to stream"));
                    close_reader(tunnel);
                    break;
                }
                
                if (written != readen) {
                    // Invalid state
                    NABTO_LOG_ERROR(("Impossible state! wanted to write %i, wrote %i, unabto_said it could write %i bytes", readen, written, canWriteToStreamBytes));
                }            
            }
        }
    }
}

bool opening_socket(tunnel* tunnel) {
    char err;
    optlen len;
    int status;
    len = sizeof(err);
    status = getsockopt(tunnel->tunnel_type_vars.tcp.sock, SOL_SOCKET, SO_ERROR, &err, &len);
    if (status != 0) {
#if defined(WINSOCK)
        NABTO_LOG_ERROR(("Error opening socket %d", WSAGetLastError()));
#else
        NABTO_LOG_ERROR(("Error opening socket %s", strerror(errno)));
#endif
        return false;
    } else {
        if (err == 0) {
            tunnel->state = TS_FORWARD;
        } else {
#if defined(WINSOCK)
        NABTO_LOG_ERROR(("Error opening socket %s", WSAGetLastError())); 
#else      
            NABTO_LOG_ERROR(("Error opening socket %s", strerror(err)));
#endif
            return false;
        }
    }
    return true;
}



bool open_socket(tunnel* tunnel) {
    struct sockaddr_in sin;
    if (-1 == (tunnel->tunnel_type_vars.tcp.sock = socket(AF_INET, SOCK_STREAM, 0))) {
        NABTO_LOG_ERROR(("Failed to create socket"));
        return false;
    }

// On windows we make a blocking accept since a nonblocking accept fails.
#ifndef WINSOCK
    {
        int flags = fcntl(tunnel->tunnel_type_vars.tcp.sock, F_GETFL, 0);
        if (flags < 0) return false;
        flags = (flags|O_NONBLOCK);
        if (fcntl(tunnel->tunnel_type_vars.tcp.sock, F_SETFL, flags) != 0) {
            NABTO_LOG_FATAL(("Cannot set unblocking mode"));
            return false;
        }
    }
#endif

#if NABTO_ENABLE_EPOLL
    {
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
        ev.data.ptr = tunnel;
    tunnel->epollEventType = UNABTO_EPOLL_TYPE_TCP_TUNNEL;
        epoll_ctl(unabto_epoll_fd, EPOLL_CTL_ADD, tunnel->tunnel_type_vars.tcp.sock, &ev);
    }
#endif
    
    memset(&sin, 0, sizeof (struct sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(tunnel->staticMemory->stmu.tcp_sm.host);
    sin.sin_port = htons(tunnel->tunnel_type_vars.tcp.port);
    if (-1 == connect(tunnel->tunnel_type_vars.tcp.sock, (struct sockaddr*) &sin, sizeof (struct sockaddr_in))) {
#if defined(WINSOCK)
        int err = WSAGetLastError();
        if (err == WSAEINPROGRESS || err == WSAEWOULDBLOCK) {
            tunnel->state = TS_OPENING_SOCKET;
        } else {
            NABTO_LOG_ERROR(("connect failed: %d", WSAGetLastError()));
            return false;
        }
#else
        int err = errno;
        if (err == EINPROGRESS || err == EWOULDBLOCK) {
            tunnel->state = TS_OPENING_SOCKET;
        } else {
            NABTO_LOG_ERROR(("connect failed: %s", strerror(err)));
            return false;
        }
#endif
    } else {
        tunnel->state = TS_FORWARD;
    }

#ifdef WINSOCK
    {
        unsigned long flags = 1;
        if (ioctlsocket(tunnel->tunnel_type_vars.tcp.sock, FIONBIO, &flags) != 0) {
            NABTO_LOG_ERROR(("Cannot set unblocking mode"));        
            return false;
        }
    }
#endif

    return true;
}

/**
 * read from unabto, write to tcp
 */
void unabto_forward_tcp(tunnel* tunnel) {
    if (tunnel->unabtoReadState == FS_WRITE) {
        tunnel->unabtoReadState = FS_READ;
    }
    while(true) {
        if (tunnel->unabtoReadState == FS_READ) {
            const uint8_t* buf;
            unabto_stream_hint hint;
            size_t readen = unabto_stream_read(tunnel->stream, &buf, &hint);
            if (hint != UNABTO_STREAM_HINT_OK) {
                close_stream_reader(tunnel);
                break;
            } else {
                if (readen == 0) {
                    break;
                } else {
                    ssize_t written;
                    NABTO_LOG_TRACE(("Write to tcp stream %i", readen));
                    written = send(tunnel->tunnel_type_vars.tcp.sock, buf, readen, MSG_NOSIGNAL);

                    if (written > 0) {
                        NABTO_LOG_TRACE(("Wrote to tcp stream %i", written));
                        unabto_stream_ack(tunnel->stream, buf, written, &hint);
                        if (hint != UNABTO_STREAM_HINT_OK) {
                            close_stream_reader(tunnel);
                            break;
                        }
                    } else if (written == 0) {
                        tunnel->unabtoReadState = FS_WRITE;
                        break;
                    } else { // -1
#if defined(WINSOCK)
                        NABTO_LOG_TRACE(("Wrote to tcp stream %i, status %d", written, WSAGetLastError()));
#else
                        NABTO_LOG_TRACE(("Wrote to tcp stream %i, status %s", written, strerror(errno)));
#endif

#if defined(WINSOCK)
                        if (WSAGetLastError() == WSAEWOULDBLOCK) {
                            tunnel->unabtoReadState = FS_WRITE;
                            break;
#else
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            tunnel->unabtoReadState = FS_WRITE;
                            break;
#endif
                        } else {
                            close_stream_reader(tunnel);
                            break;
                        }
                    }
                }
            }
        }

        if (tunnel->unabtoReadState == FS_WRITE) {
            break;
        }

        if (tunnel->unabtoReadState == FS_CLOSING) {
            break;
        }
    }
}
