/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _W5100_NETWORK_H_
#define _W5100_NETWORK_H_

#include <device_drivers/w5100/w5100_types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef w5100_socket nabto_socket_t;
#define NABTO_INVALID_SOCKET W5100_INVALID_SOCKET

void network_initialize(const uint8_t* mac);
void network_tick(void);
bool network_get_current_ip_address(uint32_t* ip);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
