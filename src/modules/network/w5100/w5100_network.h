/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _W5100_NETWORK_H_
#define _W5100_NETWORK_H_

#include <device_drivers/w5100/w5100_types.h>

#ifdef __cplusplus
extern "C" {
#endif

//typedef w5100_socket nabto_socket_t;
typedef struct nabto_socket_t nabto_socket_t;

enum nabto_socket_type {
    NABTO_SOCKET_IP_V4,
    NABTO_SOCKET_IP_V6
};

struct nabto_socket_t {
    w5100_socket sock;
    enum nabto_socket_type type;
};

#define NABTO_INVALID_SOCKET W5100_INVALID_SOCKET

void network_initialize(const uint8_t* mac);
void network_tick(void);
bool network_get_current_ip_address(uint32_t* ip);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
