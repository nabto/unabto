 
#include <unabto/unabto_stream.h>
#include <unabto/unabto_memory.h>
#include <unabto/unabto_util.h>
#include <errno.h>
#include <fcntl.h>
#include <modules/network/epoll/unabto_epoll.h>

#include <modules/tunnel_common/unabto_tunnel_common.h>
#include <modules/tunnel_common/unabto_tunnel_tcp.h>

// Defining MSG_NOSIGNAL 
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#ifdef WIN32
#define SHUT_WR SD_BOTH
#define _SCL_SECURE_NO_WARNINGS
#define close closesocket
typedef int optlen;
#else
typedef socklen_t optlen;
#endif

#if NABTO_ENABLE_EPOLL
#include <sys/epoll.h>
#endif

static const char* tunnel_host = UNABTO_TUNNEL_TCP_DEFAULT_HOST;
static uint16_t tunnel_port = UNABTO_TUNNEL_TCP_DEFAULT_PORT;


void unabto_tunnel_tcp_close_stream_reader(tunnel* tunnel);
void unabto_tunnel_tcp_close_tcp_reader(tunnel* tunnel);
void unabto_tunnel_tcp_closing(tunnel* tunnel, tunnel_event_source event_source);


void unabto_tunnel_tcp_init(tunnel* tunnel)
{
    tunnel->tunnel_type_vars.tcp.sock = INVALID_SOCKET;
}

void unabto_tunnel_tcp_event(tunnel* tunnel, tunnel_event_source event_source)
{
    if (tunnel->state == TS_OPENING_SOCKET && event_source == TUNNEL_EVENT_SOURCE_TCP_WRITE) {
        if (!opening_socket(tunnel)) {
            tunnel->state = TS_CLOSING;
        }
    }
    
    if (tunnel->state == TS_OPEN_SOCKET) {
        if (!open_socket(tunnel)) {
            tunnel->state = TS_CLOSING;
        }
    }
    if (tunnel->state == TS_FORWARD) {
        tcp_forward(tunnel);
        unabto_forward_tcp(tunnel);
    }
    
    if (tunnel->extReadState == FS_CLOSING && tunnel->unabtoReadState == FS_CLOSING) {
        tunnel->state = TS_CLOSING;
    }
    
    if (tunnel->state == TS_CLOSING) {
        unabto_tunnel_tcp_closing(tunnel, event_source);
    }
}

void unabto_tunnel_tcp_closing(tunnel* tunnel, tunnel_event_source event_source)
{
    const uint8_t* buf;
    unabto_stream_hint hint;
    size_t readen;
    
    do {
        readen = unabto_stream_read(tunnel->stream, &buf, &hint);
        if (readen > 0) {
            unabto_stream_ack(tunnel->stream, buf, readen, &hint);
        }
    } while (readen > 0);
    
    if (unabto_stream_close(tunnel->stream)) {
        unabto_stream_stats info;
        unabto_stream_get_stats(tunnel->stream, &info);
        
        NABTO_LOG_TRACE(("Closed tunnel successfully"));
        NABTO_LOG_INFO(("Tunnel(%i) closed, sentPackets: %u, sentBytes %u, sentResentPackets %u, receivedPackets %u, receivedBytes %u, receivedResentPackets %u, reorderedOrLostPackets %u", 
                        tunnel->tunnelId,
                        info.sentPackets, info.sentBytes, info.sentResentPackets,
                        info.receivedPackets, info.receivedBytes, info.receivedResentPackets, info.reorderedOrLostPackets));

#if NABTO_ENABLE_EPOLL
        {
            struct epoll_event ev;
            memset(&ev, 0, sizeof(struct epoll_event));
            epoll_ctl(unabto_epoll_fd, EPOLL_CTL_DEL, tunnel->tunnel_type_vars.tcp.sock, &ev);
        }
#endif
        
        close(tunnel->tunnel_type_vars.tcp.sock);
        unabto_stream_release(tunnel->stream);
        unabto_tunnel_reset_tunnel_struct(tunnel);
    }
}

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
            unabto_tunnel_tcp_close_tcp_reader(tunnel);
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
                unabto_tunnel_tcp_close_tcp_reader(tunnel);
                break;
            } else if (readen < 0) {
#ifdef WIN32 // defined(WINSOCK)
                if (WSAGetLastError() == WSAEWOULDBLOCK) {
                    break;
#else
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
#endif
                } else {
                    unabto_tunnel_tcp_close_tcp_reader(tunnel);
                    break;
                }
            } else if (readen > 0) {
            
                unabto_stream_hint hint;
                size_t written = unabto_stream_write(tunnel->stream, readBuffer, readen, &hint);
                if (hint != UNABTO_STREAM_HINT_OK) {
                    NABTO_LOG_TRACE(("Can't write to stream"));
                    unabto_tunnel_tcp_close_tcp_reader(tunnel);
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
#ifdef WIN32 // defined(WINSOCK)
        NABTO_LOG_ERROR(("Error opening socket %d", WSAGetLastError()));
#else
        NABTO_LOG_ERROR(("Error opening socket %s", strerror(errno)));
#endif
        return false;
    } else {
        if (err == 0) {
            tunnel->state = TS_FORWARD;
        } else {
#ifdef WIN32 // defined(WINSOCK)
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
#ifndef WIN32 //def WINSOCK
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
#ifdef WIN32 // defined(WINSOCK)
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

#ifdef WIN32 //def WINSOCK
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
                unabto_tunnel_tcp_close_stream_reader(tunnel);
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
                            unabto_tunnel_tcp_close_stream_reader(tunnel);
                            break;
                        }
                    } else if (written == 0) {
                        tunnel->unabtoReadState = FS_WRITE;
                        break;
                    } else { // -1
#ifdef WIN32 // defined(WINSOCK)
                        NABTO_LOG_TRACE(("Wrote to tcp stream %i, status %d", written, WSAGetLastError()));
#else
                        NABTO_LOG_TRACE(("Wrote to tcp stream %i, status %s", written, strerror(errno)));
#endif

#ifdef WIN32 // defined(WINSOCK)
                        if (WSAGetLastError() == WSAEWOULDBLOCK) {
                            tunnel->unabtoReadState = FS_WRITE;
                            break;
                        }
#else
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            tunnel->unabtoReadState = FS_WRITE;
                            break;
                        }
#endif
                        unabto_tunnel_tcp_close_stream_reader(tunnel);
                        break;
                        
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

#define PORT_KW_TXT "port="
#define HOST_KW_TXT "host="

void unabto_tunnel_tcp_parse_command(tunnel* tunnel, tunnel_event_source event_source)
{
    char* s;

    if (NULL != (s = strstr((const char*)tunnel->staticMemory->command, PORT_KW_TXT)))
    {
        s += strlen(PORT_KW_TXT);
        if (1 != sscanf(s, "%d", &tunnel->tunnel_type_vars.tcp.port)) {
            NABTO_LOG_ERROR(("failed to read port number"));
            tunnel->state = TS_FAILED_COMMAND;
            return;
        }
    } else {
        tunnel->tunnel_type_vars.tcp.port = unabto_tunnel_tcp_get_default_port();
    }
    
    if (NULL != (s = strstr((const char*)tunnel->staticMemory->command, HOST_KW_TXT)))
    {
        char *sp;
        int length;
        s += strlen(HOST_KW_TXT);
        sp = strchr(s, ' ');
        
        if (sp != NULL) {
            length = sp-s;
        } else {
            length = strlen(s);
        }
        
        strncpy(tunnel->staticMemory->stmu.tcp_sm.host, s, MIN(length, MAX_COMMAND_LENGTH-1));
    } else {
        strncpy(tunnel->staticMemory->stmu.tcp_sm.host, unabto_tunnel_tcp_get_default_host(), MAX_HOST_LENGTH);
    }
    tunnel->tunnelType = TUNNEL_TYPE_TCP;

    if (tunnel_allow_connection(tunnel->staticMemory->stmu.tcp_sm.host, tunnel->tunnel_type_vars.tcp.port)) {
        tunnel->state = TS_OPEN_SOCKET;
        NABTO_LOG_INFO(("Tunnel(%i) connecting to %s:%i", tunnel->tunnelId, tunnel->staticMemory->stmu.tcp_sm.host, tunnel->tunnel_type_vars.tcp.port));
    } else {
        NABTO_LOG_ERROR(("Tunnel(%i) not allowed to connect to %s:%i", tunnel->tunnelId, tunnel->staticMemory->stmu.tcp_sm.host, tunnel->tunnel_type_vars.tcp.port));
        tunnel->state = TS_FAILED_COMMAND;
    }
}

void unabto_tunnel_tcp_set_default_host(const char* host) {
    tunnel_host = host;
}

void unabto_tunnel_tcp_set_default_port(uint16_t port) {
    tunnel_port = port;
}

const char* unabto_tunnel_tcp_get_default_host() {
    return tunnel_host;
}

uint16_t unabto_tunnel_tcp_get_default_port() {
    return tunnel_port;
}

void unabto_tunnel_tcp_close_stream_reader(tunnel* tunnel) {
    NABTO_LOG_INFO(("closing socket %i", tunnel->tunnel_type_vars.tcp.sock));
    tunnel->unabtoReadState = FS_CLOSING;
    shutdown(tunnel->tunnel_type_vars.tcp.sock, SHUT_WR);
}

// no more data will come from the tcp connection to the stream
void unabto_tunnel_tcp_close_tcp_reader(tunnel* tunnel) {
    NABTO_LOG_TRACE(("close reader"));
    unabto_stream_close(tunnel->stream);
    tunnel->extReadState = FS_CLOSING;
}
