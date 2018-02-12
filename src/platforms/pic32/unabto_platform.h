/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_ENV_BASE_PIC32_H_
#define _UNABTO_ENV_BASE_PIC32_H_

#include "HardwareProfile.h"
#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "TCPIP Stack/TCPIP.h"

#include "unabto_platform_types.h"

#include <platforms/unabto_common_types.h>

//typedef UDP_SOCKET nabto_socket_t;

//#define NABTO_INVALID_SOCKET INVALID_UDP_SOCKET

/* /\** for convenience and portability *\/ */
/* typedef unsigned short uint16_t; */

/* /\** for convenience and portability *\/ */
/* typedef unsigned char uint8_t; */


//#define nabto_init_socket microchip_udp_open

//#define nabto_close_socket microchip_udp_close

//#define nabto_read microchip_udp_read

//#define nabto_write microchip_udp_write

#include "modules/network/microchip/unabto_microchip_network.h"

// Include your favorite timing module in the config file
//#include "devices/microchip/unabto_time_mchip.h"
// or
//#include "modules/timers/freertos/unabto_time_freertos.h

#define NABTO_FATAL_EXIT do { SoftReset(); } while (0)

#ifdef __cplusplus
extern "C" {
#endif

extern APP_CONFIG AppConfig; // expose the TCP/IP configuration structure so the application can access it.


#ifdef __cplusplus
} //extern "C"
#endif

#endif
