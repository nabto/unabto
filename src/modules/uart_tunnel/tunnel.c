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

#if NABTO_ENABLE_EPOLL
#include <sys/epoll.h>
#endif


#ifdef __MACH__
#define MSG_NOSIGNAL 0
#endif


#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


#define MAX_COMMAND_LENGTH 512
#define MAX_DEVICE_NAME_LENGTH 128

typedef struct tunnel_static_memory {
    uint8_t command[MAX_COMMAND_LENGTH];
    char deviceName[MAX_DEVICE_NAME_LENGTH];
    uint8_t uartReadBuffer[NABTO_MEMORY_STREAM_SEND_SEGMENT_SIZE];
} tunnel_static_memory;

NABTO_THREAD_LOCAL_STORAGE tunnel* tunnels = 0;
NABTO_THREAD_LOCAL_STORAGE tunnel_static_memory* tunnels_static_memory = 0;

static int tunnelCounter = 0;

void tunnel_event(tunnel* state, tunnel_event_source event_source);

bool open_uart(tunnel* tunnel);

void uart_forward(tunnel* tunnel);
void unabto_forward(tunnel* tunnel);

void tunnel_event_socket(int socket);
bool parse_command(tunnel* tunnel);

char* tunnel_get_default_device_name() {
    return "/dev/tty42";
}

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
        NABTO_LOG_ERROR(("Tunnel(%i), Event on tunnel which should not be in IDLE state. source %i, uartReadState %i, unabtoReadState %i, stream index %i, fd %i", tunnel->tunnelId, event_source, tunnel->uartReadState, tunnel->unabtoReadState, unabto_stream_index(tunnel->stream), tunnel->fd));
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
            tunnel->state = TS_OPEN_UART;
            NABTO_LOG_INFO(("Tunnel(%i) connecting to %s", tunnel->tunnelId, tunnel->staticMemory->deviceName));
        } else {
            NABTO_LOG_ERROR(("Tunnel(%i) Could not parse tunnel command %s", tunnel->tunnelId, tunnel->staticMemory->command));
            tunnel->state = TS_CLOSING;
            
        }
    }

    if (tunnel->state == TS_OPEN_UART) {
        // TODO make async
        if (!open_uart(tunnel)) {
            tunnel->state = TS_CLOSING;
        }
    }

    

    if (tunnel->state == TS_FORWARD) {
        if (tunnel->uartReadState == FS_CLOSING && tunnel->unabtoReadState == FS_CLOSING) {
            tunnel->state = TS_CLOSING;
        } else {
            uart_forward(tunnel);
            unabto_forward(tunnel);
        }
    }

    if (tunnel->uartReadState == FS_CLOSING && tunnel->unabtoReadState == FS_CLOSING) {
        tunnel->state = TS_CLOSING;
    }


    if (tunnel->state == TS_CLOSING) {
        NABTO_LOG_TRACE(("TS_CLOSING"));
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
            
            close(tunnel->fd);
            unabto_stream_release(tunnel->stream);
            reset_tunnel_struct(tunnel);
        }
    }
}

#define DEVICE_NAME_KW_TXT "dev="

bool parse_command(tunnel* tunnel) {
    
    char* s;

    if (NULL != (s = strstr((const char*)tunnel->staticMemory->command, DEVICE_NAME_KW_TXT)))
    {
        char *sp;
        int length;
        s += strlen(DEVICE_NAME_KW_TXT);
        sp = strchr(s, ' ');
        
        if (sp != NULL) {
            length = sp-s;
        } else {
            length = strlen(s);
        }
        
        strncpy(tunnel->staticMemory->deviceName, s, MIN(length, MAX_COMMAND_LENGTH-1));
    } else {
        strncpy(tunnel->staticMemory->deviceName, tunnel_get_default_device_name(), MAX_DEVICE_NAME_LENGTH);
    }
    
    return true;
}


void close_uart_reader(tunnel* tunnel) {
    NABTO_LOG_TRACE(("close uart reader"));
    unabto_stream_close(tunnel->stream);
    tunnel->uartReadState = FS_CLOSING;
}

/**
 * read from uart, write to unabto
 */
void uart_forward(tunnel* tunnel) {
    while(true) {
        if (tunnel->uartReadState == FS_READ) {
            ssize_t readen = read(tunnel->fd, tunnel->staticMemory->uartReadBuffer, NABTO_MEMORY_STREAM_SEND_SEGMENT_SIZE);
            if (readen == 0) {
                break;
            }
            /* if (readen == 0) { */
            /*     // eof */
            /*     NABTO_LOG_TRACE(("uart eof")); */
            /*     close_uart_reader(tunnel); */
            /*     break; */
            /* } */

            if (readen > 0) {
                tunnel->uartReadBufferSize = readen;
                tunnel->uartReadBufferSent = 0;
                tunnel->uartReadState = FS_WRITE;
            }

            if (readen < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                } else {
                    close_uart_reader(tunnel);
                    break;
                }
            }
        }

        if (tunnel->uartReadState == FS_WRITE) {
            unabto_stream_hint hint;
            size_t written = unabto_stream_write(tunnel->stream, tunnel->staticMemory->uartReadBuffer+tunnel->uartReadBufferSent, tunnel->uartReadBufferSize - tunnel->uartReadBufferSent, &hint);
            if (hint != UNABTO_STREAM_HINT_OK) {
                NABTO_LOG_TRACE(("Can't write to stream"));
                close_uart_reader(tunnel);
                break;
            } else {
                if (written > 0) {
                    tunnel->uartReadBufferSent += written;
                    if (tunnel->uartReadBufferSize == tunnel->uartReadBufferSent) {
                        tunnel->uartReadState = FS_READ;
                    }
                } else {
                    break;
                }
            }
        }

        if (tunnel->uartReadState == FS_CLOSING) {
            break;
        }
    }
}

void close_stream_reader(tunnel* tunnel) {
    tunnel->unabtoReadState = FS_CLOSING;
    NABTO_LOG_INFO(("closing fd %i", tunnel->fd));
    close(tunnel->fd);
}

/**
 * read from unabto, write to uart
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
                    NABTO_LOG_TRACE(("Write to uart stream %i", readen));
                    written = write(tunnel->fd, buf, readen);

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

bool open_uart(tunnel* tunnel) {
    struct termios tios;
    
    tunnel->fd = open(tunnel->staticMemory->deviceName, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (tunnel->fd == -1) {
        NABTO_LOG_FATAL(("cannot open uart"));
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
    
    if (-1 == tcsetattr(tunnel->fd, TCSANOW, &tios))
	{
		NABTO_LOG_FATAL(("Unable to configure UART '%s'. error: %s", tunnel->staticMemory->deviceName, strerror(errno)));
	}
    
#if NABTO_ENABLE_EPOLL
    {
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
        ev.data.ptr = tunnel;
        epoll_ctl(unabto_epoll_fd, EPOLL_CTL_ADD, tunnel->fd, &ev);
    }
#endif

    
    tunnel->state = TS_FORWARD;
    return true;
}

void reset_tunnel_struct(tunnel* t) {
    ptrdiff_t offset = t - tunnels;
    memset(t, 0, sizeof(struct tunnel));
    t->staticMemory = &tunnels_static_memory[offset];
    memset(t->staticMemory, 0, sizeof(tunnel_static_memory));
    t->fd = -1;
#if NABTO_ENABLE_EPOLL
    t->epollEventType = UNABTO_EPOLL_TYPE_UART_TUNNEL;
#endif
}


