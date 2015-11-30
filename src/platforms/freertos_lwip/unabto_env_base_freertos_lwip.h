/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
/**
 * Base env defs. for the uNabto FreeRTOS/LWIP server
 * @author jbk@nabto.com and the Nabto team
 * @since 2011-09-12
 */
#ifndef _UNABTO_ENV_BASE_LWIP_FREERTOS_H_
#define _UNABTO_ENV_BASE_LWIP_FREERTOS_H_

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "lwip/api.h"
#include "platforms/unabto_common_types.h"
#include "modules/timers/freertos/unabto_time_freertos.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  The lwIP socket library is a little overkill for our usage, so we start out by 
 * using a netconn for each socket
 */
typedef int nabto_socket_t;
typedef int16_t ssize_t;

#define NABTO_INVALID_SOCKET NULL

//#define INADDR_ANY IPADDR_ANY 
//#define INADDR_NONE IPADDR_NONE

// #define printf(...) 

#define NABTO_LOG_BASIC_PRINT(s, m) do { printf m; printf("\n");  } while(0)

#ifdef __cplusplus
} //extern "C"
#endif

#endif
