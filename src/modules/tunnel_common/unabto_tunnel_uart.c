//#ifndef WIN32
#include <unabto/unabto_stream.h>
#include <unabto/unabto_memory.h>
#include <errno.h>
#include <fcntl.h>
#include <modules/network/epoll/unabto_epoll.h>

#include <termios.h>

#include <modules/tunnel_common/unabto_tunnel_common.h>
#include <modules/tunnel_common/unabto_tunnel_uart.h>

#if NABTO_ENABLE_EPOLL
#include <sys/epoll.h>
#endif

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

static const char* defaultUartDevice = 0;

void uart_tunnel_set_default_device(const char* device) {
    defaultUartDevice = device;
}
const char* uart_tunnel_get_default_device() {
    return defaultUartDevice;
}

static void unabto_tunnel_uart_closing(tunnel* tunnel, tunnel_event_source event_source);
    
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

void unabto_tunnel_uart_event(tunnel* tunnel, tunnel_event_source event_source)
{
    if (tunnel->state == TS_FORWARD) {
        if (tunnel->extReadState == FS_CLOSING && tunnel->unabtoReadState == FS_CLOSING) {
            tunnel->state = TS_CLOSING;
        } else {
            uart_forward(tunnel);
            unabto_forward_uart(tunnel);
        }
    }
    
    if (tunnel->state == TS_CLOSING) {
        unabto_tunnel_uart_closing(tunnel, event_source);
    }
}


void unabto_tunnel_uart_closing(tunnel* tunnel, tunnel_event_source event_source)
{
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

        if (tunnel->tunnel_type_vars.uart.fd != -1) {
            close(tunnel->tunnel_type_vars.uart.fd);
            tunnel->tunnel_type_vars.uart.fd = -1;
        }

        unabto_stream_release(tunnel->stream);
        unabto_tunnel_reset_tunnel_struct(tunnel);
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
                    close_reader(tunnel);
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
                unabto_tunnel_uart_close_stream_reader(tunnel);
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
                            unabto_tunnel_uart_close_stream_reader(tunnel);
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
                            unabto_tunnel_uart_close_stream_reader(tunnel);
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

void unabto_tunnel_uart_parse_command(tunnel* t, tunnel_event_source event_source, tunnel* tunnels, size_t tunnelsLength)
{
    if(uart_tunnel_get_default_device() == 0){
        NABTO_LOG_ERROR(("No UART device was specified."));
        tunnel_send_init_message(t, "-no uart device specified\n");
        t->state = TS_FAILED_COMMAND;
        return;
    }
    strncpy(t->staticMemory->stmu.uart_sm.deviceName, uart_tunnel_get_default_device(), MAX_DEVICE_NAME_LENGTH);
    t->tunnelType = TUNNEL_TYPE_UART;
    
    unabto_tunnel_uart_steal_port(t, tunnels, tunnelsLength);
    if (open_uart(t)) {
        t->state = TS_FORWARD;
        NABTO_LOG_INFO(("Tunnel(%i) connecting with UART to %s", t->tunnelId, t->staticMemory->stmu.uart_sm.deviceName));
        tunnel_send_init_message(t, "+\n");
    } else {
        tunnel_send_init_message(t, "-cannot open uart\n");
        t->state = TS_CLOSING;
    }
}

// We have got a new stream, if there was another one using this uart
// port disconnect that user such that this tunnel can use the port
void unabto_tunnel_uart_steal_port(tunnel* tun, tunnel* tunnels, size_t tunnelsLength) {
    size_t i;
    for(i = 0; i < tunnelsLength; i++) {
        // we should not look at ourselves
        if (&tunnels[i] != tun) {
            tunnel* t = &tunnels[i];
            if (t->state == TS_FORWARD &&
                (strcmp(tun->staticMemory->stmu.uart_sm.deviceName, t->staticMemory->stmu.uart_sm.deviceName) == 0))
            {
                // close the other tunnels
                NABTO_LOG_INFO(("tunnel %i is stealing the serial port %s from tunnel %i", tun->tunnelId, tun->staticMemory->stmu.uart_sm.deviceName, t->tunnelId));
                close_reader(t);
                unabto_tunnel_uart_close_stream_reader(t);
                t->state = TS_CLOSING;
            }
        }
    }
}

void unabto_tunnel_uart_close_stream_reader(tunnel* tunnel)
{
    tunnel->unabtoReadState = FS_CLOSING;
    NABTO_LOG_INFO(("closing fd %i", tunnel->tunnel_type_vars.uart.fd));
    if (tunnel->tunnel_type_vars.uart.fd != -1) {
        close(tunnel->tunnel_type_vars.uart.fd);
        tunnel->tunnel_type_vars.uart.fd = -1;
    }
}
