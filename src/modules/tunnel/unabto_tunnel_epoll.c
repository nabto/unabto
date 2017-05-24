#include <sys/epoll.h>

#include <modules/network/epoll/unabto_epoll.h>
#include <modules/timers/auto_update/unabto_time_auto_update.h>

#include <modules/network/select/unabto_network_select_api.h>
#include <modules/tcp_fallback/tcp_fallback_select.h>
#include <unabto/unabto_common_main.h>
#include <unabto/unabto_stream.h>
#include "unabto_tunnel.h"

#include <sys/epoll.h>

#include <errno.h>
#include <stdio.h>

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
        unabto_time_update_stamp();
        unabto_next_event(&ne);
        now = nabtoGetStamp();
        timeout = nabtoStampDiff2ms(nabtoStampDiff(&ne, &now));
        if (timeout < 0) {
            timeout = 0;
        }

        fflush(stdout);
        //NABTO_LOG_INFO(("epoll wait %i", timeout));
        nfds = epoll_wait(unabto_epoll_fd, events, MAX_EPOLL_EVENTS, timeout);
        
        for (i = 0; i < nfds; i++) {
            unabto_time_update_stamp();
            unabto_epoll_event_handler* handler = (unabto_epoll_event_handler*)events[i].data.ptr;

            if (handler->epollEventType == UNABTO_EPOLL_TYPE_UDP) {
                while (unabto_network_epoll_read_one(&events[i])) {
                    unabto_time_update_stamp();
                    unabto_time_event();
                }
            }
#if NABTO_ENABLE_TCP_FALLBACK
            unabto_tcp_fallback_epoll_event(&events[i]);
#endif
            unabto_tunnel_epoll_event(&events[i]);
            
        }
        unabto_time_update_stamp();
        unabto_time_event();
    }
    deinit_tunnel_module();
    unabto_close();
}
