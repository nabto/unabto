/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_TIME_FREERTOS_H_
#define _UNABTO_TIME_FREERTOS_H_

#include "FreeRTOS.h"

typedef portTickType nabto_stamp_t;
typedef portTickType nabto_stamp_diff_t;

/*
#if (sizeof(nabto_stamp_t) != 4)
#error nabto timestamp must be 32 bit
#endif
*/

#define nabtoMsec2Stamp(msec) (msec / portTICK_PERIOD_MS)

#endif
