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

// use a bsd api
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>

#if NABTO_ENABLE_EPOLL
#include <sys/epoll.h>
#endif

#ifdef __MACH__
#define MSG_NOSIGNAL 0
#endif


#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define PORT_KW_TXT "port="
#define HOST_KW_TXT "host="

NABTO_THREAD_LOCAL_STORAGE tunnel* tunnels = 0;
NABTO_THREAD_LOCAL_STORAGE tunnel_static_memory* tunnels_static_memory = 0;

static int tunnelCounter = 0;

static const char* defaultUartDevice = 0;

/* Shared functions */
void tunnel_event(tunnel* state, tunnel_event_source event_source);
bool parse_command(tunnel* tunnel);
void ts_idle(tunnel* tunnel, tunnel_event_source event_source);
void ts_read_command(tunnel* tunnel);
void ts_parse_command(tunnel* tunnel, tunnel_event_source event_source);
void ts_forward_command(tunnel* tunnel, tunnel_event_source event_source);
void ts_closing(tunnel* tunnel, tunnel_event_source event_source);


/* Uart tunnel specific functions */

void steal_uart_port(tunnel* tunnel);

void uart_tunnel_set_default_device(const char* device) {
    defaultUartDevice = device;
}
const char* uart_tunnel_get_default_device() {
    return defaultUartDevice;
}


/* TCP tunnel specific function */


/* ECHO tunnel specific functions */



bool init_tunnel_module()
{
    int i;
    tunnels = (tunnel*)malloc(sizeof(struct tunnel) * NABTO_MEMORY_STREAM_MAX_STREAMS);
    if (tunnels == NULL) {
        return false;
    }

    tunnels_static_memory = (tunnel_static_memory*)malloc(sizeof(tunnel_static_memory) * NABTO_MEMORY_STREAM_MAX_STREAMS);
    if (tunnels_static_memory == NULL) {
        return false;
    }

    for (i = 0; i < NABTO_MEMORY_STREAM_MAX_STREAMS; i++) {
        reset_unknown_tunnel_struct(&tunnels[i]);
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
    reset_unknown_tunnel_struct(t);

    t->stream = stream;
    t->state = TS_READ_COMMAND;
    t->tunnelId = tunnelCounter++;
}

void unabto_stream_event(unabto_stream* stream, unabto_stream_event_type event) {
    tunnel* state;
    //~ NABTO_LOG_INFO(("Unabto_stream_event: %i",event));
    NABTO_LOG_TRACE(("Stream %i, %i", unabto_stream_index(stream), event));
    state = &tunnels[unabto_stream_index(stream)];
    tunnel_event(state, TUNNEL_EVENT_SOURCE_UNABTO);
}



void tunnel_event(tunnel* tunnel, tunnel_event_source event_source) {
    //~ NABTO_LOG_INFO(("tunnel event with event_source: %i, tunnel state: %i",event_source,tunnel->state));
    NABTO_LOG_TRACE(("Tunnel event on tunnel %i", tunnel));
    
    if (tunnel->state == TS_IDLE) {
        ts_idle(tunnel, event_source);
        return;
    }
    if (tunnel->state == TS_READ_COMMAND) {
        ts_read_command(tunnel);
    }
    if (tunnel->state == TS_PARSE_COMMAND) {
        ts_parse_command(tunnel, event_source);
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
        ts_forward_command(tunnel, event_source);
    }
    if (tunnel->extReadState == FS_CLOSING && tunnel->unabtoReadState == FS_CLOSING) {
        tunnel->state = TS_CLOSING;
    }
    if (tunnel->state == TS_CLOSING) {
        ts_closing(tunnel, event_source);
    }
}


void ts_idle(tunnel* tunnel, tunnel_event_source event_source){
    if (tunnel->tunnelType == TUNNEL_TYPE_UART){
        NABTO_LOG_ERROR(("Tunnel(%i), Event on tunnel which should not be in IDLE state. source %i, uartReadState %i, unabtoReadState %i, stream index %i, fd %i", tunnel->tunnelId, event_source, tunnel->extReadState, tunnel->unabtoReadState, unabto_stream_index(tunnel->stream), tunnel->tunnel_type_vars.uart.fd));
    } else if (tunnel->tunnelType == TUNNEL_TYPE_TCP){
        NABTO_LOG_ERROR(("Tunnel(%i), Event on tunnel which should not be in IDLE state. source %i, tcpReadState %i, unabtoReadState %i, stream index %i, socket %i", tunnel->tunnelId, event_source, tunnel->extReadState, tunnel->unabtoReadState, unabto_stream_index(tunnel->stream), tunnel->tunnel_type_vars.tcp.sock));
    } else if (tunnel->tunnelType == TUNNEL_TYPE_ECHO){
        // ECHO
    } else {
        NABTO_LOG_ERROR(("Tunnel(%i), Event on tunnel which should not be in IDLE state. source %i, unabtoReadState %i, stream index %i, Tunnel type unrecognized.", tunnel->tunnelId, event_source,tunnel->unabtoReadState, unabto_stream_index(tunnel->stream)));
    }
}
void ts_read_command(tunnel* tunnel){
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
void ts_parse_command(tunnel* tunnel, tunnel_event_source event_source){
    if (parse_command(tunnel)) {
        if (tunnel->tunnelType == TUNNEL_TYPE_UART){
            steal_uart_port(tunnel);
            if (open_uart(tunnel)) {
                tunnel->state = TS_FORWARD;
                NABTO_LOG_INFO(("Tunnel(%i) connecting with UART to %s", tunnel->tunnelId, tunnel->staticMemory->stmu.uart_sm.deviceName));
                tunnel_send_init_message(tunnel, "+\n");
            } else {
                tunnel_send_init_message(tunnel, "-cannot open uart\n");
                tunnel->state = TS_CLOSING;
            }
        } else if (tunnel->tunnelType == TUNNEL_TYPE_TCP){
            if (tunnel_allow_connection(tunnel->staticMemory->stmu.tcp_sm.host, tunnel->tunnel_type_vars.tcp.port)) {
                tunnel->state = TS_OPEN_SOCKET;
                NABTO_LOG_INFO(("Tunnel(%i) connecting with TCP to %s:%i", tunnel->tunnelId, tunnel->staticMemory->stmu.tcp_sm.host, tunnel->tunnel_type_vars.tcp.port));
            } else {
                NABTO_LOG_ERROR(("Tunnel(%i) not allowed to connect to %s:%i", tunnel->tunnelId, tunnel->staticMemory->stmu.tcp_sm.host, tunnel->tunnel_type_vars.tcp.port));
                tunnel->state = TS_CLOSING;
            }
            
        } else if  (tunnel->tunnelType == TUNNEL_TYPE_ECHO){
            tunnel_send_init_message(tunnel, "+\n");
            tunnel->state = TS_FORWARD;
        } else{
            NABTO_LOG_ERROR(("Tunnel(%i), Event in TS_PARSE_COMMAND state with Tunnel type unrecognized. source %i, unabtoReadState %i, stream index %i.", tunnel->tunnelId, event_source,tunnel->unabtoReadState, unabto_stream_index(tunnel->stream)));
            tunnel->state = TS_CLOSING;
        }        
    } else {
        NABTO_LOG_ERROR(("Tunnel(%i) Could not parse tunnel command %s", tunnel->tunnelId, tunnel->staticMemory->command));
        tunnel_send_init_message(tunnel, "-cannot parse command\n");
        tunnel->state = TS_CLOSING;
    }
    
}
void ts_forward_command(tunnel* tunnel, tunnel_event_source event_source){
    if (tunnel->extReadState == FS_CLOSING && tunnel->unabtoReadState == FS_CLOSING) {
        tunnel->state = TS_CLOSING;
    } else {
        if (tunnel->tunnelType == TUNNEL_TYPE_UART){
            uart_forward(tunnel);
            unabto_forward_uart(tunnel);
        } else if (tunnel->tunnelType == TUNNEL_TYPE_TCP){
            tcp_forward(tunnel);
            unabto_forward_tcp(tunnel);
        } else if  (tunnel->tunnelType == TUNNEL_TYPE_ECHO){
            echo_forward(tunnel);
        } else{
            NABTO_LOG_ERROR(("Tunnel(%i), Event in TS_FORWARD state with Tunnel type unrecognized. source %i, unabtoReadState %i, stream index %i.", tunnel->tunnelId, event_source,tunnel->unabtoReadState, unabto_stream_index(tunnel->stream)));
            tunnel->state = TS_CLOSING;
        }
    }
}
void ts_closing(tunnel* tunnel, tunnel_event_source event_source){
    const uint8_t* buf;
    unabto_stream_hint hint;
    size_t readen;

    NABTO_LOG_TRACE(("TS_CLOSING"));

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

        if (tunnel->tunnelType == TUNNEL_TYPE_UART){
            if (tunnel->tunnel_type_vars.uart.fd != -1) {
                close(tunnel->tunnel_type_vars.uart.fd);
                tunnel->tunnel_type_vars.uart.fd = -1;
            }
        } else if (tunnel->tunnelType == TUNNEL_TYPE_TCP){
            close(tunnel->tunnel_type_vars.tcp.sock);
        } else if  (tunnel->tunnelType == TUNNEL_TYPE_ECHO){
            // ECHO
        } else{
            NABTO_LOG_ERROR(("Tunnel(%i), Event in TS_CLOSING state with Tunnel type unrecognized. source %i, unabtoReadState %i, stream index %i.", tunnel->tunnelId, event_source,tunnel->unabtoReadState, unabto_stream_index(tunnel->stream)));
            tunnel->state = TS_CLOSING;
        }

        unabto_stream_release(tunnel->stream);
        reset_tunnel_struct(tunnel);
    }
}


//#define DEVICE_NAME_KW_TXT "dev="

bool parse_command(tunnel* tunnel) {
    
    if (0 == memcmp(tunnel->staticMemory->command, UART_COMMAND,strlen(UART_COMMAND))){
        if(uart_tunnel_get_default_device() == 0){
            NABTO_LOG_FATAL(("No UART device was specified."));
        }
        strncpy(tunnel->staticMemory->stmu.uart_sm.deviceName, uart_tunnel_get_default_device(), MAX_DEVICE_NAME_LENGTH);
        tunnel->tunnelType = TUNNEL_TYPE_UART;
        return true;    
    } else if (0 == memcmp(tunnel->staticMemory->command, TCP_COMMAND,strlen(TCP_COMMAND))){
        return unabto_tunnel_tcp_parse_command(tunnel);
    } else if (0 == memcmp(tunnel->staticMemory->command, ECHO_COMMAND,strlen(ECHO_COMMAND))){
        tunnel->tunnelType = TUNNEL_TYPE_ECHO;
        return true;
    } else {
        NABTO_LOG_INFO(("Failed to parse command: %s",tunnel->staticMemory->command));
        return false;
    }

    return false;

}



void close_stream_reader(tunnel* tunnel) {
    tunnel->unabtoReadState = FS_CLOSING;
    
    if(tunnel->tunnelType == TUNNEL_TYPE_UART){
        NABTO_LOG_INFO(("closing fd %i", tunnel->tunnel_type_vars.uart.fd));
        if (tunnel->tunnel_type_vars.uart.fd != -1) {
            close(tunnel->tunnel_type_vars.uart.fd);
            tunnel->tunnel_type_vars.uart.fd = -1;
        }
    } else if (tunnel->tunnelType == TUNNEL_TYPE_TCP){
        NABTO_LOG_INFO(("closing socket %i", tunnel->tunnel_type_vars.tcp.sock));
        shutdown(tunnel->tunnel_type_vars.tcp.sock, SHUT_WR);
    } else {
        //ECHO
    }
}





void reset_tunnel_struct(tunnel* t) {
    ptrdiff_t offset = t - tunnels;
    int typeTmp = t->tunnelType;
    memset(t, 0, sizeof(struct tunnel));
    t->staticMemory = &tunnels_static_memory[offset];
    memset(t->staticMemory, 0, sizeof(tunnel_static_memory));
    
    
    if(typeTmp == TUNNEL_TYPE_UART){
        t->tunnel_type_vars.uart.fd = -1;
#if NABTO_ENABLE_EPOLL
        t->epollEventType = UNABTO_EPOLL_TYPE_UART_TUNNEL;
#endif
    } else if (typeTmp == TUNNEL_TYPE_TCP){
        t->tunnel_type_vars.tcp.sock = INVALID_SOCKET;
#if NABTO_ENABLE_EPOLL
        t->epollEventType = UNABTO_EPOLL_TYPE_TCP_TUNNEL;
#endif
    } else {
        // ECHO TUNNEL
    }
}

void reset_unknown_tunnel_struct(tunnel* t) {
    ptrdiff_t offset = t - tunnels;
    memset(t, 0, sizeof(struct tunnel));
    t->staticMemory = &tunnels_static_memory[offset];
    memset(t->staticMemory, 0, sizeof(tunnel_static_memory));
}


// We have got a new stream, if there was another one using this uart
// port disconnect that user such that this tunnel can use the port
void steal_uart_port(tunnel* tun) {
    int i;
    for(i = 0; i < NABTO_MEMORY_STREAM_MAX_STREAMS; i++) {
        // we should not look at ourselves
        if (&tunnels[i] != tun) {
            tunnel* t = &tunnels[i];
            if (t->state == TS_FORWARD &&
                (strcmp(tun->staticMemory->stmu.uart_sm.deviceName, t->staticMemory->stmu.uart_sm.deviceName) == 0))
            {
                // close the other tunnels
                NABTO_LOG_INFO(("tunnel %i is stealing the serial port %s from tunnel %i", tun->tunnelId, tun->staticMemory->stmu.uart_sm.deviceName, t->tunnelId));
                close_reader(t);
                close_stream_reader(t);
                t->state = TS_CLOSING;
            }
        }
    }
}
