#include <unabto/unabto_stream.h>
#include <unabto/unabto_app.h>
#include <unabto/unabto_common_main.h>
#include <unabto/unabto_main_contexts.h>
#include <unabto/unabto_external_environment.h>
#include <modules/network/epoll/unabto_epoll.h>
#include <modules/tcp_fallback/tcp_fallback_select.h>

#include <string.h>

#include "tunnel.h"

#if WINSOCK
#else
// use a bsd api
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#if NABTO_ENABLE_EPOLL
#include <sys/epoll.h>
#endif


#ifdef __MACH__
#define MSG_NOSIGNAL 0
#endif


#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


#define MAX_COMMAND_LENGTH 512
#define MAX_HOST_LENGTH 128

typedef struct tunnel_static_memory {
    uint8_t command[MAX_COMMAND_LENGTH];
    char host[MAX_HOST_LENGTH];
    uint8_t tcpReadBuffer[NABTO_STREAM_SEND_SEGMENT_SIZE];
} tunnel_static_memory;

NABTO_THREAD_LOCAL_STORAGE tunnel tunnels[NABTO_STREAM_MAX_STREAMS];
NABTO_THREAD_LOCAL_STORAGE tunnel_static_memory tunnels_static_memory[NABTO_STREAM_MAX_STREAMS];

static int tunnelCounter = 0;

void tunnel_event(tunnel* state, tunnel_event_source event_source);

bool open_socket(tunnel* tunnel);

void tcp_forward(tunnel* tunnel);
void unabto_forward(tunnel* tunnel);

bool add_socket(int sock);

void remove_socket(int sock);

void tunnel_event_socket(int socket);
bool parse_command(tunnel* tunnel);
bool opening_socket(tunnel* tunnel);



void unabto_stream_accept(unabto_stream* stream) {
    tunnel* t = &tunnels[unabto_stream_index(stream)];
    NABTO_LOG_TRACE(("Accepting stream and assigning it to tunnel %i", t));
    UNABTO_ASSERT(t->state == TS_IDLE);
    reset_tunnel_struct(t);

    t->stream = stream;
    t->state = TS_READ_COMMAND;
    t->tunnelId = tunnelCounter++;
}

void unabto_stream_event(unabto_stream* stream, unabto_stream_event_type event) {
    tunnel* state;
    NABTO_LOG_TRACE(("Stream %i, %i", unabto_stream_index(stream), event));
    state = &tunnels[unabto_stream_index(stream)];
    tunnel_event(state, TUNNEL_EVENT_SOURCE_UNABTO);
}

void tunnel_event(tunnel* tunnel, tunnel_event_source event_source) {

    NABTO_LOG_TRACE(("Tunnel event on tunnel %i", tunnel));
    if (tunnel->state == TS_IDLE) {
        NABTO_LOG_ERROR(("Tunnel(%i), Event on tunnel which should not be in IDLE state. source %i, tcpReadState %i, unabtoReadState %i, stream index %i, socket %i", tunnel->tunnelId, event_source, tunnel->tcpReadState, tunnel->unabtoReadState, unabto_stream_index(tunnel->stream), tunnel->sock));
        return;
    }

    if (tunnel->state == TS_READ_COMMAND) {
        const uint8_t* buf;
        unabto_stream_hint hint;
        size_t readen = unabto_stream_read(tunnel->stream, &buf, &hint);
        if (hint != UNABTO_STREAM_HINT_OK) {
            NABTO_LOG_TRACE(("Releasing stream in state TS_READ_COMMAND"));
            tunnel->state = TS_CLOSING;
        } else {
            if (readen > 0) {
                size_t i;
                for (i = 0; i < readen; i++) {
                    if (buf[i] == '\n') {
                        tunnel->state = TS_PARSE_COMMAND;
                    } else {
                        tunnel->staticMemory->command[tunnel->commandLength] = buf[i];
                        tunnel->commandLength++;
                    }
                    
                    if (tunnel->commandLength > MAX_COMMAND_LENGTH) {
                        NABTO_LOG_ERROR(("Tunnel command too long"));
                        tunnel->state = TS_CLOSING;
                    }
                }
                
                unabto_stream_ack(tunnel->stream, buf, i, &hint);

                if (hint != UNABTO_STREAM_HINT_OK) {
                    NABTO_LOG_ERROR(("Failed to ack on stream."));
                    tunnel->state = TS_CLOSING;
                }
            }
        }
    }

    if (tunnel->state == TS_PARSE_COMMAND) {
        if (parse_command(tunnel)) {
            if (tunnel_allow_connection(tunnel->staticMemory->host, tunnel->port)) {
                tunnel->state = TS_OPEN_SOCKET;
                NABTO_LOG_INFO(("Tunnel(%i) connecting to %s:%i", tunnel->tunnelId, tunnel->staticMemory->host, tunnel->port));
            } else {
                NABTO_LOG_ERROR(("Tunnel(%i) not allowed to connect to %s:%i", tunnel->tunnelId, tunnel->staticMemory->host, tunnel->port));
                tunnel->state = TS_CLOSING;
            }
        } else {
            NABTO_LOG_ERROR(("Tunnel(%i) Could not parse tunnel command %s", tunnel->tunnelId, tunnel->staticMemory->command));
            tunnel->state = TS_CLOSING;
            
        }
    }

    if (tunnel->state == TS_OPENING_SOCKET && event_source == TUNNEL_EVENT_SOURCE_TCP_WRITE) {
        if (!opening_socket(tunnel)) {
            tunnel->state = TS_CLOSING;
        }
    }

    if (tunnel->state == TS_OPEN_SOCKET) {
        // TODO make async
        if (!open_socket(tunnel)) {
            tunnel->state = TS_CLOSING;
        }
    }

    

    if (tunnel->state == TS_FORWARD) {
        if (tunnel->tcpReadState == FS_CLOSING && tunnel->unabtoReadState == FS_CLOSING) {
            tunnel->state = TS_CLOSING;
        } else {
            tcp_forward(tunnel);
            unabto_forward(tunnel);
        }
    }

    if (tunnel->tcpReadState == FS_CLOSING && tunnel->unabtoReadState == FS_CLOSING) {
        tunnel->state = TS_CLOSING;
    }


    if (tunnel->state == TS_CLOSING) {
        
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

            close(tunnel->sock);
            unabto_stream_release(tunnel->stream);
            reset_tunnel_struct(tunnel);
        }
    }
}

#define PORT_KW_TXT "port="
#define HOST_KW_TXT "host="

bool parse_command(tunnel* tunnel) {
    
    char* s;

    if (NULL != (s = strstr((const char*)tunnel->staticMemory->command, PORT_KW_TXT)))
    {
        s += strlen(PORT_KW_TXT);
        if (1 != sscanf(s, "%d", &tunnel->port)) {
            NABTO_LOG_ERROR(("failed to read port number"));
            return false;
        }
    } else {
        tunnel->port = tunnel_get_default_port();
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
        
        strncpy(tunnel->staticMemory->host, s, MIN(length, MAX_COMMAND_LENGTH-1));
    } else {
        strncpy(tunnel->staticMemory->host, tunnel_get_default_host(), MAX_HOST_LENGTH);
    }
    
    return true;
}


void close_tcp_reader(tunnel* tunnel) {
    unabto_stream_close(tunnel->stream);
    tunnel->tcpReadState = FS_CLOSING;
}

/**
 * read from tcp, write to unabto
 */
void tcp_forward(tunnel* tunnel) {
    while(true) {
        if (tunnel->tcpReadState == FS_READ) {
            ssize_t readen = recv(tunnel->sock, tunnel->staticMemory->tcpReadBuffer, NABTO_STREAM_SEND_SEGMENT_SIZE, 0);
            if (readen == 0) {
                // eof
                close_tcp_reader(tunnel);
                break;
            }

            if (readen > 0) {
                tunnel->tcpReadBufferSize = readen;
                tunnel->tcpReadBufferSent = 0;
                tunnel->tcpReadState = FS_WRITE;
            }

            if (readen < 0) {
#if defined(WINSOCK)
                if (WSAGetLastError() == WSAEWOULDBLOCK) {
                    break;
#else
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
#endif
                } else {
                    close_tcp_reader(tunnel);
                    break;
                }
            }
        }

        if (tunnel->tcpReadState == FS_WRITE) {
            unabto_stream_hint hint;
            size_t written = unabto_stream_write(tunnel->stream, tunnel->staticMemory->tcpReadBuffer+tunnel->tcpReadBufferSent, tunnel->tcpReadBufferSize - tunnel->tcpReadBufferSent, &hint);
            if (hint != UNABTO_STREAM_HINT_OK) {
                NABTO_LOG_TRACE(("Can't write to stream"));
                close_tcp_reader(tunnel);
                break;
            } else {
                if (written > 0) {
                    tunnel->tcpReadBufferSent += written;
                    if (tunnel->tcpReadBufferSize == tunnel->tcpReadBufferSent) {
                        tunnel->tcpReadState = FS_READ;
                    }
                } else {
                    break;
                }
            }
        }

        if (tunnel->tcpReadState == FS_CLOSING) {
            break;
        }
    }
}

void close_stream_reader(tunnel* tunnel) {
    tunnel->unabtoReadState = FS_CLOSING;
    shutdown(tunnel->sock, SHUT_WR);
}

/**
 * read from unabto, write to tcp
 */
void unabto_forward(tunnel* tunnel) {
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
                    written = send(tunnel->sock, buf, readen, MSG_NOSIGNAL);

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

bool opening_socket(tunnel* tunnel) {
    char err;
    optlen len;
    int status;
    len = sizeof(err);
    status = getsockopt(tunnel->sock, SOL_SOCKET, SO_ERROR, &err, &len);
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
    if (-1 == (tunnel->sock = socket(AF_INET, SOCK_STREAM, 0))) {
        NABTO_LOG_ERROR(("Failed to create socket"));
        return false;
    }

// On windows we make a blocking accept since a nonblocking accept fails.
#ifndef WINSOCK
    {
        int flags = fcntl(tunnel->sock, F_GETFL, 0);
        if (flags < 0) return false;
        flags = (flags|O_NONBLOCK);
        if (fcntl(tunnel->sock, F_SETFL, flags) != 0) {
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
        epoll_ctl(unabto_epoll_fd, EPOLL_CTL_ADD, tunnel->sock, &ev);
    }
#endif
    
    memset(&sin, 0, sizeof (struct sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(tunnel->staticMemory->host);
    sin.sin_port = htons(tunnel->port);
    if (-1 == connect(tunnel->sock, (struct sockaddr*) &sin, sizeof (struct sockaddr_in))) {
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
        if (ioctlsocket(tunnel->sock, FIONBIO, &flags) != 0) {
            NABTO_LOG_ERROR(("Cannot set unblocking mode"));        
            return false;
        }
    }
#endif

    return true;
}

void reset_tunnel_struct(tunnel* t) {
    ptrdiff_t offset = t - tunnels;
    memset(t, 0, sizeof(struct tunnel));
    t->staticMemory = &tunnels_static_memory[offset];
    memset(t->staticMemory, 0, sizeof(tunnel_static_memory));
    t->sock = INVALID_SOCKET;
#if NABTO_ENABLE_EPOLL
    t->epollEventType = UNABTO_EPOLL_TYPE_TCP_TUNNEL;
#endif
}

 
const char* tunnel_host = DEFAULT_HOST;
int tunnel_port = DEFAULT_PORT;


void tunnel_set_default_host(const char* host) {
    tunnel_host = host;
}

void tunnel_set_default_port(int port) {
    tunnel_port = port;
}

const char* tunnel_get_default_host() {
    return tunnel_host;
}

int tunnel_get_default_port() {
    return tunnel_port;
}
