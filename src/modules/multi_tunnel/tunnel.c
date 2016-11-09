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

bool open_uart(tunnel* tunnel);
void uart_forward(tunnel* tunnel);
void unabto_forward_uart(tunnel* tunnel);
bool tunnel_send_init_message(tunnel* tunnel, const char* msg);
void steal_uart_port(tunnel* tunnel);
void uart_tunnel_set_default_device(const char* device) {
    defaultUartDevice = device;
}
const char* uart_tunnel_get_default_device() {
    return defaultUartDevice;
}


/* TCP tunnel specific function */
bool opening_socket(tunnel* tunnel);
bool open_socket(tunnel* tunnel);
void tcp_forward(tunnel* tunnel);
void unabto_forward_tcp(tunnel* tunnel);
bool add_socket(int sock);
void remove_socket(int sock);

/* ECHO tunnel specific functions */
void echo_forward(tunnel* tunnel);


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
	strncpy(tunnel->staticMemory->stmu.uart_sm.deviceName, uart_tunnel_get_default_device(), MAX_DEVICE_NAME_LENGTH);
	tunnel->tunnelType = TUNNEL_TYPE_UART;
	return true;	
    } else if (0 == memcmp(tunnel->staticMemory->command, TCP_COMMAND,strlen(TCP_COMMAND))){
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
	tunnel->tunnelType = TUNNEL_TYPE_TCP;
	return true;
    } else if (0 == memcmp(tunnel->staticMemory->command, ECHO_COMMAND,strlen(ECHO_COMMAND))){
	tunnel->tunnelType = TUNNEL_TYPE_ECHO;
    } else {
	NABTO_LOG_INFO(("Failed to parse command: %s",tunnel->staticMemory->command));
	return false;
    }
	
    //~ if (0 != strcmp(tunnel->staticMemory->uart_sm.command, UART_COMMAND)
	//~ && 0 != strcmp(tunnel->staticMemory->uart_sm.command, TCP_COMMAND)
	//~ && 0 != strcmp(tunnel->staticMemory->uart_sm.command, ECHO_COMMAND)) {
        //~ // the string has to start with uart, tcp, or echo
        //~ return false;
    //~ }


}


void close_uart_reader(tunnel* tunnel) {
    NABTO_LOG_TRACE(("close uart reader"));
    unabto_stream_close(tunnel->stream);
    tunnel->extReadState = FS_CLOSING;
}

void close_tcp_reader(tunnel* tunnel) {
    unabto_stream_close(tunnel->stream);
    tunnel->extReadState = FS_CLOSING;
}

/**
 * Echo unabto back to unabto
 */
void echo_forward(tunnel* tunnel){
 
    const uint8_t* buf;
    unabto_stream_hint hint;
    size_t readLength = unabto_stream_read(tunnel->stream, &buf, &hint);

    if (readLength > 0) {
	size_t writeLength = unabto_stream_write(tunnel->stream, buf, readLength, &hint);
	if (writeLength > 0) {
	    if (!unabto_stream_ack(tunnel->stream, buf, writeLength, &hint)) {
		tunnel->state = TS_CLOSING;
	    }
	} else {
	    if (hint != UNABTO_STREAM_HINT_OK) {
		tunnel->state = TS_CLOSING;
	    }
	}
    } else {
	if (hint !=  UNABTO_STREAM_HINT_OK) {
	    tunnel->state = TS_CLOSING;
	}
    }

}


/**
 * read from uart, write to unabto
 */
void uart_forward(tunnel* tunnel) {
    while(true) {
        unabto_stream_hint hint;
        size_t canWriteToStreamBytes;

        if (tunnel->extReadState == FS_CLOSING) {
            break;
        }
        
        canWriteToStreamBytes = unabto_stream_can_write(tunnel->stream, &hint);
        if (hint != UNABTO_STREAM_HINT_OK) {
            close_uart_reader(tunnel);
            break;
        }
        if (canWriteToStreamBytes == 0) {
            tunnel->extReadState = FS_WRITE;
            break;
        } else {
            tunnel->extReadState = FS_READ;
        }
        {
            int readen;
            uint8_t readBuffer[NABTO_MEMORY_STREAM_SEND_SEGMENT_SIZE];
            size_t maxRead = MIN(NABTO_MEMORY_STREAM_SEND_SEGMENT_SIZE, canWriteToStreamBytes);

            readen = read(tunnel->tunnel_type_vars.uart.fd, readBuffer, maxRead);
            if (readen == 0) {
                // no data available
                break;
            }
            
            if (readen < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                } else {
                    close_uart_reader(tunnel);
                    break;
                }
            }
            
            if (readen > 0) {
                unabto_stream_hint hint;
                size_t written = unabto_stream_write(tunnel->stream, readBuffer, readen, &hint);
                // the earlier call to unabto_stream_can_write
                // ensures we can write all the bytes to the
                // stream in one write
                if (hint != UNABTO_STREAM_HINT_OK) {
                    NABTO_LOG_TRACE(("Can't write to stream"));
                    close_uart_reader(tunnel);
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
            close_tcp_reader(tunnel);
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
                close_tcp_reader(tunnel);
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
                    close_tcp_reader(tunnel);
                    break;
                }
            } else if (readen > 0) {
            
                unabto_stream_hint hint;
                size_t written = unabto_stream_write(tunnel->stream, readBuffer, readen, &hint);
                if (hint != UNABTO_STREAM_HINT_OK) {
                    NABTO_LOG_TRACE(("Can't write to stream"));
                    close_tcp_reader(tunnel);
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


/**
 * read from unabto, write to uart
 */
void unabto_forward_uart(tunnel* tunnel) {
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
                    NABTO_LOG_TRACE(("Write to uart stream %i", readen));
                    written = write(tunnel->tunnel_type_vars.uart.fd, buf, readen);

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
                        NABTO_LOG_TRACE(("Wrote to uart stream %i, status %s", written, strerror(errno)));

                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            tunnel->unabtoReadState = FS_WRITE;
                            break;
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


bool open_uart(tunnel* tunnel) {
    struct termios tios;
    
    tunnel->tunnel_type_vars.uart.fd = open(tunnel->staticMemory->stmu.uart_sm.deviceName, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (tunnel->tunnel_type_vars.uart.fd == -1) {
        NABTO_LOG_ERROR(("cannot open uart %s", tunnel->staticMemory->stmu.uart_sm.deviceName));
        return false;
    }

    memset(&tios, 0, sizeof(tios));
    tios.c_iflag = IGNBRK | IGNCR;
    tios.c_iflag |= ((tcflag_t) INPCK);
    tios.c_oflag &= ~OPOST;
    tios.c_lflag &= ~ICANON;
    cfmakeraw(&tios); // :...
    tios.c_cc[VMIN] = 0;
    tios.c_cc[VTIME] = 0;
    tios.c_cflag = CLOCAL | CREAD;
        
    
    /* switch(parity) */
    /* { */
	/* 	case UART_PARITY_NONE: */
	/* 		break; */
			
	/* 	case UART_PARITY_EVEN: */
	/* 		tios.c_cflag |= ((tcflag_t) PARENB); */
	/* 		break; */
			
	/* 	case UART_PARITY_ODD: */
	/* 		tios.c_cflag |= ((tcflag_t) PARENB | PARODD); */
	/* 		break; */
			
	/* 	// case UART_PARITY_MARK: */
	/* 		// tios.c_cflag |= ((tcflag_t) PARENB | CMSPAR | PARODD); */
	/* 		// break; */
			
	/* 	// case UART_PARITY_SPACE: */
	/* 		// tios.c_cflag |= ((tcflag_t) PARENB | CMSPAR); */
	/* 		// break; */

	/* 	default: */
	/* 		NABTO_LOG_FATAL(("Invalid number of databits for UART!")); */
    /* } */

    /* switch(stopbits) */
    /* { */
	/* 	case UART_STOPBITS_ONE: */
	/* 		break; */

	/* 	case UART_STOPBITS_TWO: */
	/* 		tios.c_cflag |= ((tcflag_t) CSTOPB); */
	/* 		break; */

	/* 	default: */
	/* 		NABTO_LOG_FATAL(("Invalid number of stopbits for UART!")); */
    /* } */
	
	/* switch(databits) */
	/* { */
	/* 	case 5: */
	/* 		tios.c_cflag |= ((tcflag_t) CS5); */
	/* 		break; */
	/* 	case 6: */
	/* 		tios.c_cflag |= ((tcflag_t) CS6); */
	/* 		break; */
	/* 	case 7: */
	/* 		tios.c_cflag |= ((tcflag_t) CS7); */
	/* 		break; */
	/* 	case 8: */
	/* 		tios.c_cflag |= ((tcflag_t) CS8); */
	/* 		break; */
	/* 	default: */
	/* 		NABTO_LOG_FATAL(("Invalid number of databits specified for UART '%s'.", _name)); */
	/* 		return; */
	/* } */
    
    /* switch(baudrate) */
	/* { */
	/* 	case 300: */
	/* 		cfsetispeed(&tios, B300); */
	/* 		cfsetospeed(&tios, B300); */
	/* 	case 600: */
	/* 		cfsetispeed(&tios, B600); */
	/* 		cfsetospeed(&tios, B600); */
	/* 	case 1200: */
	/* 		cfsetispeed(&tios, B1200); */
	/* 		cfsetospeed(&tios, B1200); */
	/* 	case 2400: */
	/* 		cfsetispeed(&tios, B2400); */
	/* 		cfsetospeed(&tios, B2400); */
	/* 		break; */
	/* 	case 4800: */
	/* 		cfsetispeed(&tios, B4800); */
	/* 		cfsetospeed(&tios, B4800); */
	/* 		break; */
	/* 	case 9600: */
	/* 		cfsetispeed(&tios, B9600); */
	/* 		cfsetospeed(&tios, B9600); */
	/* 		break; */
	/* 	case 19200: */
	/* 		cfsetispeed(&tios, B19200); */
	/* 		cfsetospeed(&tios, B19200); */
	/* 		break; */
	/* 	case 38400: */
	/* 		cfsetispeed(&tios, B38400); */
	/* 		cfsetospeed(&tios, B38400); */
	/* 		break; */
	/* 	case 57600: */
	/* 		cfsetispeed(&tios, B57600); */
	/* 		cfsetospeed(&tios, B57600); */
	/* 		break; */
	/* 	case 115200: */
	/* 		cfsetispeed(&tios, B115200); */
	/* 		cfsetospeed(&tios, B115200); */
	/* 		break; */
	/* 	default: */
	/* 		NABTO_LOG_FATAL(("Invalid baudrate specified for UART '%s'.", _name)); */
	/* 		return; */
	/* } */

    // defautlt settings for now
    
    tios.c_cflag |= ((tcflag_t) CS8);
    if (-1 == cfsetispeed(&tios, B115200)) {
        NABTO_LOG_ERROR(("unable to set baudrate"));
    }
    if (-1 == cfsetospeed(&tios, B115200)) {
        NABTO_LOG_ERROR(("unable to set baudrate"));
    }
    
    if (-1 == tcsetattr(tunnel->tunnel_type_vars.uart.fd, TCSANOW, &tios))
	{
		NABTO_LOG_ERROR(("Unable to configure UART '%s'. error: %s", tunnel->staticMemory->stmu.uart_sm.deviceName, strerror(errno)));
        return false;
	}
    
#if NABTO_ENABLE_EPOLL
    {
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
        ev.data.ptr = tunnel;
	tunnel->epollEventType = UNABTO_EPOLL_TYPE_UART_TUNNEL;
        epoll_ctl(unabto_epoll_fd, EPOLL_CTL_ADD, tunnel->tunnel_type_vars.uart.fd, &ev);
    }
#endif

    
    tunnel->state = TS_FORWARD;
    return true;
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
/**
 * utility function for either sending +ok\n or -error message\n in
 * case of either success or failure.  see spec in
 * https://www.rfc-editor.org/rfc/rfc1078.txt instead of CRLF only LF
 * is used.
 */
bool tunnel_send_init_message(tunnel* tunnel, const char* msg)
{
    unabto_stream_hint hint;
    size_t written;
    size_t writeLength = strlen(msg);
    written = unabto_stream_write(tunnel->stream, (uint8_t*)msg, writeLength, &hint);
    if (written != writeLength) {
        NABTO_LOG_ERROR(("we should at a minimum be able to send this simple message, we will probably just goive up..."));
        return false;
    }
    return true;
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
                close_uart_reader(t);
                close_stream_reader(t);
                t->state = TS_CLOSING;
            }
        }
    }
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
