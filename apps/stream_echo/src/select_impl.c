#include <unabto/unabto_common_main.h>
#include <modules/cli/unabto_args.h>
#include <unabto/unabto_app.h>
#include "stream_echo.h"
#include <modules/timers/auto_update/unabto_time_auto_update.h>
#include <modules/network/select/unabto_network_select_api.h>

#if NABTO_ENABLE_TCP_FALLBACK
#include <modules/tcp_fallback/tcp_fallback_select.h>
#endif

#ifdef WIN32
#include <windows.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/resource.h>
#endif

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

void wait_event();

int select_start(nabto_main_setup* nms) {
    stream_echo_init();
    
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

void wait_event()
{
    int timeout;
    nabto_stamp_t ne;
    nabto_stamp_t now;
    int nfds;
    int max_read_fd = 0;
    int max_write_fd = 0;
    fd_set read_fds;
    fd_set write_fds;
    struct timeval timeout_val;
    
    unabto_next_event(&ne);
    now = nabtoGetStamp();
    timeout = nabtoStampDiff2ms(nabtoStampDiff(&ne, &now));
    if (timeout < 0) timeout = 0;

    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);

    
    unabto_network_select_add_to_read_fd_set(&read_fds, &max_read_fd);
    
#if NABTO_ENABLE_TCP_FALLBACK
    unabto_tcp_fallback_select_add_to_read_fd_set(&read_fds, &max_read_fd);
    unabto_tcp_fallback_select_add_to_write_fd_set(&write_fds, &max_write_fd);
#endif

    timeout_val.tv_sec = (timeout/1000);
    timeout_val.tv_usec = ((timeout)%1000)*1000;

    nfds = select(MAX(max_read_fd+1, max_write_fd+1), &read_fds, &write_fds, NULL, &timeout_val);

    NABTO_LOG_TRACE(("foobar %i", nfds));
    if (nfds < 0) NABTO_LOG_FATAL(("Error in epoll_wait: %d", errno));
    unabto_network_select_read_sockets(&read_fds);

#if NABTO_ENABLE_TCP_FALLBACK
    unabto_tcp_fallback_select_read_sockets(&read_fds);
    unabto_tcp_fallback_select_write_sockets(&write_fds);
#endif
    
    unabto_time_event();
}

application_event_result application_event(application_request* applicationRequest, unabto_query_request* readBuffer, unabto_query_response* writeBuffer) {
    return AER_REQ_INV_QUERY_ID;
}
