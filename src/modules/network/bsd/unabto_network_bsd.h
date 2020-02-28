/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_NETWORK_BSD_H_
#define _UNABTO_NETWORK_BSD_H_

#include <unabto_platform_types.h>

#ifndef UNABTO_NETWORK_BSD_NONBLOCKING
#define UNABTO_NETWORK_BSD_NONBLOCKING 1
#endif

#ifndef NABTO_NETWORK_LOOPBACK_ONLY
#define NABTO_NETWORK_LOOPBACK_ONLY 0
#endif

typedef struct nabto_socket_t nabto_socket_t;

enum nabto_socket_type {
    NABTO_SOCKET_IP_V4,
    NABTO_SOCKET_IP_V6
};

//typedef int nabto_socket_t;
struct nabto_socket_t {
    int sock;
    enum nabto_socket_type type;
};


#define NABTO_INVALID_SOCKET -1

#ifdef __cplusplus
extern "C" {
#endif

void nabto_bsd_set_nonblocking(nabto_socket_t* socketDescriptor);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
