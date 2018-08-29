#include <unabto/unabto_common_main.h>
#include <modules/cli/unabto_args.h>
#include <unabto/unabto_app.h>
#include <unabto/unabto_memory.h>
#include "stream_echo.h"
#include <modules/timers/auto_update/unabto_time_auto_update.h>
#include <modules/network/select/unabto_network_select_api.h>
#include <modules/network/epoll/unabto_epoll.h>
#include <modules/tcp_fallback/tcp_fallback_select.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/resource.h>
#include <sys/epoll.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

void wait_event();

int main(int argc, char** argv) {

    unabto_epoll_init();
    
    nabto_main_setup* nms = unabto_init_context();

    stream_echo_init();
    
    if (!check_args(argc, argv, nms)) {
        return 1;
    }

    if (!unabto_init()) {
        return 1;
    }
    
    unabto_time_auto_update(false);
    while(true) {
        unabto_time_update_stamp();
        wait_event();
    }
    
    unabto_close();
}

#define MAX_EVENTS 16

void wait_event()
{
    struct epoll_event events[MAX_EVENTS];
    int timeout;
    nabto_stamp_t ne;
    nabto_stamp_t now;
    int nfds;
    int i;
    
    unabto_next_event(&ne);
    now = nabtoGetStamp();
    timeout = nabtoStampDiff2ms(nabtoStampDiff(&ne, &now));
    if (timeout < 0) {
        timeout = 0;
    }

    nfds = epoll_wait(unabto_epoll_fd, events, MAX_EVENTS, timeout);

    NABTO_LOG_TRACE(("foobar %i", nfds));
    if (nfds < 0) {
        if (errno == EINTR) {
            
        } else {
            NABTO_LOG_FATAL(("Error in epoll_wait: %d", errno));
        }
    }

    
    for (i = 0; i < nfds; i++) {
        unabto_epoll_event_handler* handler = (unabto_epoll_event_handler*)events[i].data.ptr;
        
        if (handler->epollEventType == UNABTO_EPOLL_TYPE_UDP) {
            unabto_epoll_event_handler_udp* udpHandler = (unabto_epoll_event_handler_udp*)handler;
            unabto_read_socket(udpHandler->fd);
        }
#if NABTO_ENABLE_TCP_FALLBACK
        if (handler->epollEventType == UNABTO_EPOLL_TYPE_TCP_FALLBACK) {
            nabto_connect* con = (nabto_connect*)handler;
            if (events[i].events & EPOLLIN) {
                unabto_tcp_fallback_read_ready(con);
            }
            if (events[i].events & EPOLLOUT) {
                unabto_tcp_fallback_write_ready(con);
            }
        }
#endif
        
    }
    
    unabto_time_event();
}

application_event_result application_event(application_request* applicationRequest, unabto_query_request* readBuffer, unabto_query_response* writeBuffer) {
    return AER_REQ_INV_QUERY_ID;
}
