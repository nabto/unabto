/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_MICROCHIP_NETWORK_H_
#define _UNABTO_MICROCHIP_NETWORK_H_

#include <modules/network/microchip/unabto_microchip_udp.h>

#define NABTO_INVALID_SOCKET        INVALID_UDP_SOCKET

typedef UDP_SOCKET nabto_socket_t;

void nabto_socket_set_invalid(nabto_socket_t* socket)
{
    socket = NABTO_INVALID_SOCKET;
}
bool nabto_socket_is_equal(const nabto_socket_t* s1, const nabto_socket_t* s2)
{
    return *s1==*s2;
}


#define nabto_socket_init           microchip_udp_open
#define nabto_socket_close          microchip_udp_close
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
