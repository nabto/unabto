#include <unabto/unabto_stream.h>
#include <unabto/unabto_app.h>
#include <unabto/unabto_memory.h>
#include <unabto/unabto_common_main.h>
#include <modules/timers/auto_update/unabto_time_auto_update.h>
#include <modules/network/select/unabto_network_select_api.h>
#include <modules/tcp_fallback/tcp_fallback_select.h>
#include <unabto/unabto_util.h>

#include "tunnel_select.h"
#include "tunnel.h"

#if defined(WIN32) || defined(WINCE)
// use winsock
#define WINSOCK 1
#else
// use a bsd api
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

void tunnel_loop_select() {
    int i;
    
    if (!unabto_init()) {
        NABTO_LOG_FATAL(("Failed to initialize unabto"));
    }

    if (!init_tunnel_module()) {
        NABTO_LOG_FATAL(("Cannot initialize tunnel module"));
        return;
    }

    unabto_time_auto_update(false);
    // time is updated here and after the select since that's the only blocking point.
    unabto_time_update_stamp();
    while(true) {
        nabto_stamp_t nextEvent;
        nabto_stamp_t now;
        int timeout;
        int i;
        fd_set read_fds;
        fd_set write_fds;
        int max_read_fd = 0;
        int max_write_fd = 0;
        struct timeval timeout_val;
        int nfds;
        
        unabto_next_event(&nextEvent);
        now = nabtoGetStamp();
        timeout = nabtoStampDiff2ms(nabtoStampDiff(&nextEvent, &now));

        if (timeout < 0) {
            timeout = 1;
        }

        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        unabto_network_select_add_to_read_fd_set(&read_fds, &max_read_fd);

#if NABTO_ENABLE_TCP_FALLBACK
        unabto_tcp_fallback_select_add_to_read_fd_set(&read_fds, &max_read_fd);
        unabto_tcp_fallback_select_add_to_write_fd_set(&write_fds, &max_write_fd);
#endif

        for (i = 0; i < NABTO_MEMORY_STREAM_MAX_STREAMS; i++) {
            if (tunnels[i].state != TS_IDLE) {
                if (tunnels[i].state == TS_FORWARD && tunnels[i].tcpReadState == FS_READ) {
                    FD_SET(tunnels[i].sock, &read_fds);
                    max_read_fd = MAX(max_read_fd, tunnels[i].sock);
                }
                if ((tunnels[i].state == TS_FORWARD && tunnels[i].unabtoReadState == FS_WRITE) ||
                    tunnels[i].state == TS_OPENING_SOCKET) {
                    FD_SET(tunnels[i].sock, &write_fds);
                    max_write_fd = MAX(max_write_fd, tunnels[i].sock);
                }
            }
        }
        timeout_val.tv_sec = (timeout/1000);
        timeout_val.tv_usec = ((timeout)%1000)*1000;

        nfds = select(MAX(max_read_fd+1, max_write_fd+1), &read_fds, &write_fds, NULL, &timeout_val);

        if (nfds < 0) {
#if defined(WINSOCK)
            if (WSAGetLastError() == WSAEINTR) {
                // ok
            } else {
                NABTO_LOG_ERROR(("Select returned error %d", WSAGetLastError()));
                return;
            }
#else
            int err = errno;
            if (err == EINTR) {
                // ok
            } else {
                NABTO_LOG_ERROR(("Select returned error %s, %i", strerror(err), err));
                return;
            }
#endif
        }

        unabto_time_update_stamp();

        if (nfds > 0) {
#if NABTO_ENABLE_TCP_FALLBACK
            unabto_tcp_fallback_select_write_sockets(&write_fds);
            unabto_tcp_fallback_select_read_sockets(&read_fds);
#endif
            unabto_network_select_read_sockets(&read_fds);
            for (i = 0; i < NABTO_MEMORY_STREAM_MAX_STREAMS; i++) {
                if (tunnels[i].sock != INVALID_SOCKET && FD_ISSET(tunnels[i].sock, &read_fds)) {
                    tunnel_event(&tunnels[i], TUNNEL_EVENT_SOURCE_TCP_READ);
                }
                if (tunnels[i].sock != INVALID_SOCKET && FD_ISSET(tunnels[i].sock, &write_fds)) {
                    tunnel_event(&tunnels[i], TUNNEL_EVENT_SOURCE_TCP_WRITE);
                }
            }
        }
        unabto_time_event();
    }
    deinit_tunnel_module();
    unabto_close();
}

