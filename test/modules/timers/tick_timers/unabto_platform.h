#ifndef UNABTO_PLATFORM_H
#define UNABTO_PLATFORM_H

#include <stdint.h>
#include <stdbool.h>

#include <modules/timers/tick_timer/unabto_tick_timer.h>

//typedef int nabto_socket_t;
typedef struct nabto_socket_t nabto_socket_t;

enum nabto_socket_type {
    NABTO_SOCKET_IP_V4,
    NABTO_SOCKET_IP_V6
};

struct nabto_socket_t {
    int sock;
    enum nabto_socket_type type;
};

#endif

