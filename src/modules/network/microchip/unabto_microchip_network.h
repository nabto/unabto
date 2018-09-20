/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_MICROCHIP_NETWORK_H_
#define _UNABTO_MICROCHIP_NETWORK_H_

#include <modules/network/microchip/unabto_microchip_udp.h>

#define NABTO_INVALID_SOCKET        INVALID_UDP_SOCKET

//typedef UDP_SOCKET nabto_socket_t;
typedef struct nabto_socket_t nabto_socket_t;

enum nabto_socket_type {
    NABTO_SOCKET_IP_V4,
    NABTO_SOCKET_IP_V6
};

struct nabto_socket_t {
    UDP_SOCKET sock;
    enum nabto_socket_type type;
};

#define nabto_init_socket           microchip_udp_open
#define nabto_close_socket          microchip_udp_close
#define nabto_read                  microchip_udp_read
#define nabto_write                 microchip_udp_write

#ifdef __cplusplus
extern "C" {
#endif

void network_initialize(void);
void network_tick(void);
bool network_get_current_ip_address(uint32_t* ip);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
