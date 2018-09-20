/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_ENV_BASE_COLDFIRE_H_
#define _UNABTO_ENV_BASE_COLDFIRE_H_

#include <devices/coldfire/types.h>

typedef int ssize_t;

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

#define NABTO_INVALID_SOCKET 0


#define htons(n) (uint16_t)(((uint16_t) (n) << 8) | (uint16_t) (n) >> 8)
#define htonl(n) (uint32_t)( ((uint32_t) htons(n) << 16) | htons((uint32_t) (n) >> 16) )
//
#define ntohs htons
#define ntohl htonl
//
//#define nabto_init_socket microchip_udp_open
//#define nabto_close_socket microchip_udp_close
//#define nabto_read microchip_udp_read
//#define nabto_write microchip_udp_write

//#define nabto_init_socket (0)
//#define nabto_close_socket
//#define nabto_read (0)
//#define nabto_write (0)
//
//#include "devices/microchip/unabto_microchip_udp.h"
//#include "devices/microchip/unabto_time_mchip.h"
//
//void nabto_dns_resolver(void);
//
//extern void strcpy_s(char* destination, size_t destinationLength, const char* source);
//extern void strcpypgm2ram_s(char* destination, size_t destinationLength, const far rom char* source);
//
//#define strcat_s(dst, dstlen, src) strcat(dst, src)

//#include "support_common.h"
#include <devices/coldfire/unabto_time_coldfire.h>

#endif
