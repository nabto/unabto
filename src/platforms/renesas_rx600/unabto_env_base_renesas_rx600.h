/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_ENV_BASE_RENESAS_RX600_H_
#define _UNABTO_ENV_BASE_RENESAS_RX600_H_

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

typedef int nabto_socket_t;
typedef long ssize_t;
typedef unsigned long size_t;

#define NABTO_INVALID_SOCKET NULL

#define INADDR_ANY  0x00000000UL 
#define INADDR_NONE 0xffffffffUL

#define NABTO_ENABLE_MALLOC 0
#define NABTO_ENABLE_STREAM 0
#define IPADDR_STRING_SIZE (4+4+4+4)

//typedef long long nabto_stamp_t;
void nabtoYield(int msec);

#define printf(...) 

#include "../../modules/timers/tick_timer/unabto_tick_timer.h"


#endif
