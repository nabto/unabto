#include <unabto/unabto_stream.h>
#include <unabto/unabto_app.h>
#include <unabto/unabto_memory.h>
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

NABTO_THREAD_LOCAL_STORAGE tunnel* tunnels = 0;
NABTO_THREAD_LOCAL_STORAGE tunnel_static_memory* tunnels_static_memory = 0;

static int tunnelCounter = 0;

void tunnel_event(tunnel* state, tunnel_event_source event_source);

bool open_socket(tunnel* tunnel);

void tunnel_event_socket(int socket);
bool parse_command(tunnel* tunnel);


bool init_tunnel_module()
{
    int i;
    tunnels = (tunnel*)malloc(sizeof(struct tunnel) * NABTO_MEMORY_STREAM_MAX_STREAMS);
    if (tunnels == NULL) {
        return false;
    }

    tunnels_static_memory = (tunnel_static_memory*)malloc(sizeof(struct tunnel_static_memory) * NABTO_MEMORY_STREAM_MAX_STREAMS);
    if (tunnels_static_memory == NULL) {
        return false;
    }

    for (i = 0; i < NABTO_MEMORY_STREAM_MAX_STREAMS; i++) {
        reset_tunnel_struct(&tunnels[i]);
    }

    return true;
}

void deinit_tunnel_module()
{
    free(tunnels); tunnels = 0;
    free(tunnels_static_memory); tunnels_static_memory = 0;
}

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
        NABTO_LOG_ERROR(("Tunnel(%i), Event on tunnel which should not be in IDLE state. source %i, tcpReadState %i, unabtoReadState %i, stream index %i, socket %i", tunnel->tunnelId, event_source, tunnel->extReadState, tunnel->unabtoReadState, unabto_stream_index(tunnel->stream), tunnel->tunnel_type_vars.tcp.sock));
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
            if (tunnel_allow_connection(tunnel->staticMemory->stmu.tcp_sm.host, tunnel->tunnel_type_vars.tcp.port)) {
                tunnel->state = TS_OPEN_SOCKET;
                NABTO_LOG_INFO(("Tunnel(%i) connecting to %s:%i", tunnel->tunnelId, tunnel->staticMemory->stmu.tcp_sm.host, tunnel->tunnel_type_vars.tcp.port));
            } else {
                NABTO_LOG_ERROR(("Tunnel(%i) not allowed to connect to %s:%i", tunnel->tunnelId, tunnel->staticMemory->stmu.tcp_sm.host, tunnel->tunnel_type_vars.tcp.port));
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
        if (tunnel->extReadState == FS_CLOSING && tunnel->unabtoReadState == FS_CLOSING) {
            tunnel->state = TS_CLOSING;
        } else {
            tcp_forward(tunnel);
            unabto_forward_tcp(tunnel);
        }
    }

    if (tunnel->extReadState == FS_CLOSING && tunnel->unabtoReadState == FS_CLOSING) {
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

            close(tunnel->tunnel_type_vars.tcp.sock);
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
        if (1 != sscanf(s, "%d", &tunnel->tunnel_type_vars.tcp.port)) {
            NABTO_LOG_ERROR(("failed to read port number"));
            return false;
        }
    } else {
        tunnel->tunnel_type_vars.tcp.port = tunnel_get_default_port();
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
        strncpy(tunnel->staticMemory->stmu.tcp_sm.host, tunnel_get_default_host(), MAX_HOST_LENGTH);
    }
    
    return true;
}


void close_stream_reader(tunnel* tunnel) {
    tunnel->unabtoReadState = FS_CLOSING;
    shutdown(tunnel->tunnel_type_vars.tcp.sock, SHUT_WR);
}


void reset_tunnel_struct(tunnel* t) {
    ptrdiff_t offset = t - tunnels;
    memset(t, 0, sizeof(struct tunnel));
    t->staticMemory = &tunnels_static_memory[offset];
    memset(t->staticMemory, 0, sizeof(tunnel_static_memory));
    t->tunnel_type_vars.tcp.sock = INVALID_SOCKET;
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
