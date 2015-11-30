#ifndef _UNABTO_EPOLL_H_
#define _UNABTO_EPOLL_H_

#include <unabto/unabto_env_base.h>

/**
 * This is the one and oly epoll fd which the main application guarantees to initialize.
 */
extern NABTO_THREAD_LOCAL_STORAGE int unabto_epoll_fd;

#define UNABTO_EPOLL_TYPE_UDP 1
#define UNABTO_EPOLL_TYPE_TCP_FALLBACK 2
#define UNABTO_EPOLL_TYPE_TCP_TUNNEL 3

typedef struct {
    int epollEventType;
} unabto_epoll_event_handler;

typedef struct {
    int epollEventType;
    int fd;
} unabto_epoll_event_handler_udp;

void unabto_epoll_init();


/**
 * default disable epoll such that we are default compatible with most
 * platforms. Since epoll requires some explicit system calls on
 * socket creation and destruction.
 */
#ifndef NABTO_ENABLE_EPOLL
#define NABTO_ENABLE_EPOLL 0
#endif

#endif
