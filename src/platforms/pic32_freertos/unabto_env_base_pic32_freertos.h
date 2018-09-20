/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_ENV_BASE_PIC32_H_
#define _UNABTO_ENV_BASE_PIC32_H_

#include "HardwareProfile.h"
#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "TCPIP Stack/TCPIP.h"
#include <stdint.h>
#include <sys/types.h>
#include <stdbool.h>
#include "devices/microchip/unabto_microchip_udp.h"
#include "devices/microchip/unabto_environment_mchip.h"

#include "FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef portTickType nabto_stamp_t;

#define nabtoMsec2Stamp(ms) ((ms) / portTICK_RATE_MS)

typedef UDP_SOCKET nabto_socket_t;

#define NABTO_INVALID_SOCKET INVALID_UDP_SOCKET

/** for convenience and portability */
typedef unsigned short uint16_t;

/** for convenience and portability */
typedef unsigned char uint8_t;

void nabto_set_invalid_socket(nabto_socket_t* socket)
{
    socket = NABTO_INVALID_SOCKET;
}

#define nabto_init_socket microchip_udp_open

#define nabto_close_socket microchip_udp_close

#define nabto_read microchip_udp_read

#define nabto_write microchip_udp_write


#ifdef __cplusplus
} //extern "C"
#endif

#endif
