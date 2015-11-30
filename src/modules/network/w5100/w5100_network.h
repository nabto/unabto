/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _W5100_NETWORK_H_
#define _W5100_NETWORK_H_

#include <device_drivers/w5100/w5100.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NABTO_INVALID_SOCKET W5100_INVALID_SOCKET
#define nabto_socket_t w5100_socket

#if UNABTO_PLATFORM_PIC18

#define nabto_init_socket w5100_nabto_init_socket
#define nabto_close_socket w5100_nabto_close_socket
#define nabto_write w5100_nabto_write
#define nabto_read w5100_nabto_read

#endif

void network_initialize(const uint8_t* mac);
void network_tick(void);
bool network_get_current_ip_address(uint32_t* ip);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
