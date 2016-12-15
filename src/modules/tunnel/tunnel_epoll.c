#include <sys/epoll.h>

#include <modules/network/epoll/unabto_epoll.h>
#include <modules/timers/auto_update/unabto_time_auto_update.h>

#include <modules/network/select/unabto_network_select_api.h>
#include <modules/tcp_fallback/tcp_fallback_select.h>
#include <unabto/unabto_common_main.h>
#include <unabto/unabto_stream.h>
#include "tunnel.h"

#include <sys/epoll.h>

#include <errno.h>

#define MAX_EPOLL_EVENTS 10

void tunnel_loop_epoll() {

    struct epoll_event events[MAX_EPOLL_EVENTS];
    int timeout;
    nabto_stamp_t ne;
    nabto_stamp_t now;
    int nfds;

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

    while (true) {
        int i;
        unabto_next_event(&ne);
        now = nabtoGetStamp();
        timeout = nabtoStampDiff2ms(nabtoStampDiff(&ne, &now));
        if (timeout < 0) {
            timeout = 0;
        }
        
        nfds = epoll_wait(unabto_epoll_fd, events, MAX_EPOLL_EVENTS, timeout);
        unabto_time_update_stamp();
        
        for (i = 0; i < nfds; i++) {
            
            unabto_epoll_event_handler* handler = (unabto_epoll_event_handler*)events[i].data.ptr;

            if (handler->epollEventType == UNABTO_EPOLL_TYPE_UDP) {
                unabto_epoll_event_handler_udp* udpHandler = (unabto_epoll_event_handler_udp*)handler;
                bool status;
                do {
                    status = unabto_read_socket(udpHandler->fd);
                } while (status);
            }
#if NABTO_ENABLE_TCP_FALLBACK
            unabto_tcp_fallback_epoll_event(&events[i]);
#endif
            if (handler->epollEventType == UNABTO_EPOLL_TYPE_TCP_TUNNEL) {
                tunnel* tunnelPtr = (tunnel*)handler;
                if (tunnelPtr->tunnel_type_vars.tcp.sock != INVALID_SOCKET) {
                    tunnel_event(tunnelPtr, TUNNEL_EVENT_SOURCE_TCP_READ);
                }
                if (tunnelPtr->tunnel_type_vars.tcp.sock != INVALID_SOCKET) {
                    tunnel_event(tunnelPtr, TUNNEL_EVENT_SOURCE_TCP_WRITE);
                }
            }
        }

        unabto_time_event();
    }
    deinit_tunnel_module();
    unabto_close();
}
