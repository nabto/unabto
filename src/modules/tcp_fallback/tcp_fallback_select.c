#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#include <io.h>
#else
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/tcp.h>
#endif
#include <sys/types.h>
#include <fcntl.h>
#include <modules/tcp_fallback/tcp_fallback_select.h>
#include <unabto/unabto_util.h>
#include <errno.h>
#include <unabto/unabto_memory.h>
#include <unabto/unabto_message.h>
#include <unabto/unabto_tcp_fallback.h>

#if NABTO_ENABLE_EPOLL
#include <sys/epoll.h>
#endif

#if NABTO_ENABLE_DYNAMIC_MEMORY
static NABTO_THREAD_LOCAL_STORAGE unabto_tcp_fallback_connection* fbConns = 0;
#else
static NABTO_THREAD_LOCAL_STORAGE unabto_tcp_fallback_connection fbConns[NABTO_MEMORY_CONNECTIONS_SIZE];
#endif

bool unabto_tcp_fallback_handle_connect(nabto_connect* con);
void unabto_tcp_fallback_read_packets(nabto_connect* con);
bool unabto_tcp_fallback_handle_write(nabto_connect* con);

void close_tcp_socket(nabto_connect* con);

bool unabto_tcp_fallback_module_init()
{
#if NABTO_ENABLE_DYNAMIC_MEMORY
    fbConns = (unabto_tcp_fallback_connection*)malloc(sizeof(unabto_tcp_fallback_connection) * NABTO_MEMORY_CONNECTIONS_SIZE);
    if (fbConns == 0) {
        NABTO_LOG_FATAL(("Could not allocate memory for fallback connections"));
        return false;
    }
#endif
    return true;
}

void unabto_tcp_fallback_deinit() {
#if NABTO_ENABLE_DYNAMIC_MEMORY
    free(fbConns); fbConns = 0;
#endif
}

void unabto_tcp_fallback_select_add_to_read_fd_set(fd_set* readFds, int* maxReadFd) {
    int i;
    for (i = 0; i < NABTO_MEMORY_CONNECTIONS_SIZE; i++) {
        nabto_connect* con = &connections[i];

        if (con->state != CS_IDLE) {
            unabto_tcp_fallback_state st = con->tcpFallbackConnectionState;
            unabto_tcp_fallback_connection* fbConn = &fbConns[nabto_connection_index(con)];
            if (st > UTFS_CONNECTING && st < UTFS_CLOSED && fbConn->socket.socket != INVALID_SOCKET) {
                FD_SET(fbConn->socket.socket, readFds);
                *maxReadFd = MAX(*maxReadFd ,(int)(fbConn->socket.socket));
            }
        }
    }
}

void unabto_tcp_fallback_select_add_to_write_fd_set(fd_set* writeFds, int* maxWriteFd) {
    int i;
    for (i = 0; i < NABTO_MEMORY_CONNECTIONS_SIZE; i++) {
        nabto_connect* con = &connections[i];

        if (con->state != CS_IDLE) {
            unabto_tcp_fallback_state st = con->tcpFallbackConnectionState;
            unabto_tcp_fallback_connection* fbConn = &fbConns[nabto_connection_index(con)];
            if (st > UTFS_IDLE && st < UTFS_CLOSED) {
                if ((st == UTFS_CONNECTING || fbConn->sendBufferLength > 0) && fbConn->socket.socket != INVALID_SOCKET) {
                    FD_SET(fbConn->socket.socket, writeFds);
                    *maxWriteFd = MAX(*maxWriteFd ,(int)(fbConn->socket.socket));
                }
            }
        }
    }
}

void unabto_tcp_fallback_read_ready(nabto_connect* con) { 
    if (con->state != CS_IDLE) {
        unabto_tcp_fallback_state st = con->tcpFallbackConnectionState;
        unabto_tcp_fallback_connection* fbConn = &fbConns[nabto_connection_index(con)];
        if (st > UTFS_IDLE && st < UTFS_CLOSED && fbConn->socket.socket != INVALID_SOCKET) {
            // this calls reads until something blocks.
            unabto_tcp_fallback_read_packets(con);
        }
    }
}


void unabto_tcp_fallback_select_read_sockets(fd_set* readFds) {
    uint16_t i;
    for (i = 0; i < NABTO_MEMORY_CONNECTIONS_SIZE; i++) {
        nabto_connect* con = &connections[i];

        if (con->state != CS_IDLE) {
            unabto_tcp_fallback_state st = con->tcpFallbackConnectionState;
            unabto_tcp_fallback_connection* fbConn = &fbConns[nabto_connection_index(con)];
            if (st > UTFS_IDLE && st < UTFS_CLOSED && fbConn->socket.socket != INVALID_SOCKET) {
                if (FD_ISSET(fbConn->socket.socket, readFds)) {
                    unabto_tcp_fallback_read_packets(con);
                }
            }
        }
    }
}

void unabto_tcp_fallback_select_write_sockets(fd_set* writeFds) {
    int i;
    for (i = 0; i < NABTO_MEMORY_CONNECTIONS_SIZE; i++) {
        nabto_connect* con = &connections[i];

        if (con->state != CS_IDLE) {
            unabto_tcp_fallback_state st = con->tcpFallbackConnectionState;
            unabto_tcp_fallback_connection* fbConn = &fbConns[nabto_connection_index(con)];
            if (st > UTFS_IDLE && st < UTFS_CLOSED && fbConn->socket.socket != INVALID_SOCKET) {
                if (FD_ISSET(fbConn->socket.socket, writeFds)) {
                    if (st == UTFS_CONNECTING) {
                        unabto_tcp_fallback_handle_connect(con);
                    } else if (st >= UTFS_CONNECTED) {
                        unabto_tcp_fallback_handle_write(con);
                    }
                } 
            }
        }
    }
}

void unabto_tcp_fallback_write_ready(nabto_connect* con) {
    if (con->state != CS_IDLE) {
        unabto_tcp_fallback_state st = con->tcpFallbackConnectionState;
        unabto_tcp_fallback_connection* fbConn = &fbConns[nabto_connection_index(con)];
        if (st > UTFS_IDLE && st < UTFS_CLOSED && fbConn->socket.socket != INVALID_SOCKET) {
            
            if (st == UTFS_CONNECTING) {
                unabto_tcp_fallback_handle_connect(con);
            }
            if (st >= UTFS_CONNECTED) {
                bool status;
                do {
                    status = unabto_tcp_fallback_handle_write(con);
                } while (status);
            } 
        }
    }
}

#if NABTO_ENABLE_EPOLL
void unabto_tcp_fallback_epoll_event(struct epoll_event* event)
{
    nabto_connect* con = (nabto_connect*)event->data.ptr;
    if (con->epollEventType == UNABTO_EPOLL_TYPE_TCP_FALLBACK) {
        if (event->events & EPOLLIN) {
            unabto_tcp_fallback_read_ready(con);
        }
        if (event->events & EPOLLOUT) {
            unabto_tcp_fallback_write_ready(con);
        }
    }
}
#endif

bool unabto_tcp_fallback_init(nabto_connect* con) {
    unabto_tcp_fallback_connection* fbConn = &fbConns[nabto_connection_index(con)];
    memset(fbConn, 0, sizeof(unabto_tcp_fallback_connection));
    fbConn->sendBufferLength = 0;
    fbConn->socket.socket = INVALID_SOCKET;
#if NABTO_ENABLE_EPOLL
    con->epollEventType = UNABTO_EPOLL_TYPE_TCP_FALLBACK;
#endif
    return true;
}

/**
 * This is called by unabto when the resource is released. All
 * resources must have been released when returning.
 */
bool unabto_tcp_fallback_close(nabto_connect* con) {
    if (con->tcpFallbackConnectionState != UTFS_CLOSED) {
        close_tcp_socket(con);
    }
    return true;
}

void unabto_tcp_fallback_read_packets(nabto_connect* con) {
    unabto_tcp_fallback_connection* fbConn = &fbConns[nabto_connection_index(con)];
    while(true) {
        if (fbConn->recvBufferLength < 16) {
            unabto_tcp_status status;
            size_t readen;

            status = unabto_tcp_read(&fbConn->socket, fbConn->recvBuffer + fbConn->recvBufferLength, 16-fbConn->recvBufferLength, &readen);

            if (status == UTS_OK) {
                fbConn->recvBufferLength+=readen;
            } else if (status == UTS_WOULD_BLOCK) {
                return;
            } else { // UTS_FAILED
                NABTO_LOG_ERROR((PRI_tcp_fb "unabto_tcp_fallback_read_single_packet failed", TCP_FB_ARGS(con)));
                return;
            }
        }

        if (fbConn->recvBufferLength >= 16) {
            uint16_t packetLength;
            unabto_tcp_status status;
            size_t readen;
            READ_U16(packetLength, fbConn->recvBuffer+14);

            status = unabto_tcp_read(&fbConn->socket, fbConn->recvBuffer + fbConn->recvBufferLength, packetLength - fbConn->recvBufferLength, &readen);

            if (status == UTS_OK) {
                if (readen == 0) {
                    NABTO_LOG_INFO((PRI_tcp_fb "TCP fallback connection closed by peer", TCP_FB_ARGS(con)));
                    unabto_tcp_fallback_close(con);
                    return;
                } else {
                    fbConn->recvBufferLength += readen;
                }
            } else if (status == UTS_WOULD_BLOCK) {
                return;
            } else { // UTS_FAILED
                NABTO_LOG_ERROR((PRI_tcp_fb "Tcp read failed", TCP_FB_ARGS(con)));
                return;
            }
            
            if (fbConn->recvBufferLength == packetLength) {
                message_event event;
                event.type = MT_TCP_FALLBACK;
                
                memcpy(nabtoCommunicationBuffer, fbConn->recvBuffer, fbConn->recvBufferLength);
                
                NABTO_LOG_TRACE((PRI_tcp_fb "Received fallback packet length %" PRIsize, TCP_FB_ARGS(con), fbConn->recvBufferLength));
                
                nabto_message_event(&event, (uint16_t)(fbConn->recvBufferLength));
                NABTO_LOG_TRACE((PRI_tcp_fb "fallback packet done\n==================================================", TCP_FB_ARGS(con)));
                
                fbConn->recvBufferLength = 0;
            }
        }
    }
}


static bool unabto_tcp_fallback_create_socket(unabto_tcp_fallback_connection* fbConn, nabto_connect* con)
{
    unabto_tcp_status status;
    status = unabto_tcp_open(&fbConn->socket, (void*) con);
    if(status == UTS_OK) {
        return true;
    } else {
        return false;
    }
}

static void unabto_tcp_fallback_close_socket(unabto_tcp_fallback_connection* fbConn)
{
    unabto_tcp_close(&fbConn->socket);
}



bool unabto_tcp_fallback_connect(nabto_connect* con) {
    unabto_tcp_fallback_connection* fbConn = &fbConns[nabto_connection_index(con)];
    int flags;
    nabto_endpoint ep;
    unabto_tcp_status status;

    if (!unabto_tcp_fallback_create_socket(fbConn, con)) {
        NABTO_LOG_ERROR((PRI_tcp_fb "Could not create socket for tcp fallback.", TCP_FB_ARGS(con)));
        return false;
    }
    ep.addr = con->fallbackHost.addr;
    ep.port = con->fallbackHost.port;
    status = unabto_tcp_connect(&fbConn->socket, &ep);

    NABTO_LOG_INFO((PRI_tcp_fb "Ep. " PRIep, TCP_FB_ARGS(con), MAKE_EP_PRINTABLE(con->fallbackHost)));

    if (status == UTS_OK) {
        con->tcpFallbackConnectionState = UTFS_CONNECTED;
    } else if (status == UTS_CONNECTING) {
        con->tcpFallbackConnectionState = UTFS_CONNECTING;
    } else { // UTS_FAILED
        NABTO_LOG_ERROR((PRI_tcp_fb "Could not connect to fallback tcp endpoint. %s", TCP_FB_ARGS(con), strerror(errno)));
        con->tcpFallbackConnectionState = UTFS_CLOSED;
        return false;
    }
    return true;
}

bool unabto_tcp_fallback_handle_connect(nabto_connect* con) {
    unabto_tcp_fallback_connection* fbConn = &fbConns[nabto_connection_index(con)];
    unabto_tcp_status status = unabto_tcp_connect_poll(&fbConn->socket);
    if (status == UTS_OK) {
        con->tcpFallbackConnectionState = UTFS_CONNECTED;
        return true;
    } else { // UTS_FAILED
        unabto_tcp_fallback_close_socket(fbConn);
        con->tcpFallbackConnectionState = UTFS_CLOSED;
        return true;
    }
}

void close_tcp_socket(nabto_connect* con) {
    unabto_tcp_fallback_connection* fbConn = &fbConns[nabto_connection_index(con)];
    unabto_tcp_fallback_close_socket(fbConn);

    con->tcpFallbackConnectionState = UTFS_CLOSED;
    unabto_tcp_fallback_socket_closed(con);
}

bool unabto_tcp_fallback_handle_write(nabto_connect* con) {
    unabto_tcp_status status;
    size_t written;
    int dataToSend;
    bool canMaybeSendMoreData = false;
    unabto_tcp_fallback_connection* fbConn = &fbConns[nabto_connection_index(con)];
    UNABTO_ASSERT(fbConn->sendBufferSent <= fbConn->sendBufferLength); 
    dataToSend = fbConn->sendBufferLength - fbConn->sendBufferSent;

    if (dataToSend == 0) {
        return false;
    }
    
    NABTO_LOG_TRACE(("data to send %i, sendBufferLength %i, sendBufferSent %i", dataToSend, fbConn->sendBufferLength, fbConn->sendBufferSent));

    status = unabto_tcp_write(&fbConn->socket, fbConn->sendBuffer + fbConn->sendBufferSent, dataToSend, &written);

    if (status == UTS_OK) {
        fbConn->sendBufferSent += written;
        canMaybeSendMoreData = true;
    } else if (status == UTS_WOULD_BLOCK) {
        canMaybeSendMoreData = false;
    } else { // UTS_FAILED
        NABTO_LOG_ERROR((PRI_tcp_fb "Send of tcp packet failed", TCP_FB_ARGS(con)));
        canMaybeSendMoreData = false;
        return canMaybeSendMoreData;
    }

    if (fbConn->sendBufferSent > fbConn->sendBufferLength) {
        NABTO_LOG_FATAL(("fbConn->sendBufferSent(%" PRIsize ") > fbConn->sendBufferLength(%" PRIsize "), that should not be possible", fbConn->sendBufferSent, fbConn->sendBufferLength));
    }

    if (fbConn->sendBufferSent == fbConn->sendBufferLength) {
        fbConn->sendBufferLength = 0;
        fbConn->sendBufferSent = 0;
        canMaybeSendMoreData = false;
    }

    NABTO_LOG_TRACE(("state after send, sendBufferLength %i, sendBufferSent %i", fbConn->sendBufferLength, fbConn->sendBufferSent));
    
    return canMaybeSendMoreData;
}


unabto_tcp_fallback_error unabto_tcp_fallback_write(nabto_connect* con, uint8_t* buffer, size_t bufferLength) {

    unabto_tcp_fallback_connection* fbConn = &fbConns[nabto_connection_index(con)];
    unabto_tcp_fallback_error status = UTFE_OK;
    NABTO_LOG_TRACE(("unabto_tcp_fallback_write bytes: %i", bufferLength));
    if (fbConn->sendBufferLength == 0) {
        memcpy(fbConn->sendBuffer, buffer, bufferLength);
        fbConn->sendBufferLength = bufferLength;
        fbConn->sendBufferSent = 0;
        unabto_tcp_fallback_handle_write(con);
        status = UTFE_OK;
        NABTO_LOG_TRACE((PRI_tcp_fb "Succesful queue of tcp fallback packet.", TCP_FB_ARGS(con)));
    } else {
        NABTO_LOG_TRACE((PRI_tcp_fb "Could not enqueue tcp fallback packet.", TCP_FB_ARGS(con)));
        status = UTFE_QUEUE_FULL;
    }

    return status;
}
