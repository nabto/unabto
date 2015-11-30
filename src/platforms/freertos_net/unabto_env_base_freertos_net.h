/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_ENV_BASE_FREERTOS_NET_H_
#define _UNABTO_ENV_BASE_FREERTOS_NET_H_

/**
 * @file
 * The Basic environment for the Nabto FreeRTOS NET Device Server interface.
 *
 * This file holds definitions, declarations and prototypes to be supplied by the host.
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOS_Sockets.h"
#include <platforms/unabto_common_types.h>
#include "modules/timers/freertos/unabto_time_freertos.h"

#include "unabto_platform_types.h"

/** for convenience and portability */
//enum { false, true };

/** for convenience and portability */
//typedef int bool;


typedef xSocket_t nabto_socket_t;
#define NABTO_INVALID_SOCKET NULL

/************ The Time Handling basics (PC Device), Interface *************/

#ifdef hest
/** The timestamp definition. @return */
typedef portTickType nabto_stamp_t;
typedef nabto_stamp_t nabto_stamp_diff_t;

/**
 * Convert milleseconds to nabto_stamp_t difference (duration)
 * @param msec  number of milliseconds
 * @return      the stamp difference
 */
#define nabtoMsec2Stamp(milliseconds)  ((milliseconds)/portTICK_RATE_MS) 
#endif
/**
 * Remove thread/process from active queue (if relevant)
 * @param msec  the number of milliseconds before retrieving work.
 *
 * This functionality may only be needed in polling/pseudo-polling environments.
 */
void nabtoYield(int msec);

nabto_socket_t *getActiveSockets(uint16_t *nSockets);

#endif
