#include "unabto_epoll.h"

#include <sys/epoll.h>

NABTO_THREAD_LOCAL_STORAGE int unabto_epoll_fd;

void unabto_epoll_init() {
    unabto_epoll_fd = epoll_create(42 /** unused */);
}
