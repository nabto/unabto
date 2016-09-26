/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_MICROCHIP_TIME_H_
#define _UNABTO_MICROCHIP_TIME_H_

#include <unabto_platform_types.h>
#include <TCPIP Stack/Tick.h>

/**
 * We measure time in ticks.
 */
typedef uint32_t nabto_stamp_t;
typedef int32_t  nabto_stamp_diff_t;

// This is assuming TICK_SECOND >> 1000
// On default pic32 it is 238609
#define TICK_MILLISECOND ( TICK_SECOND / 1000ul )

/**
 * Convert milliseconds to nabto_stamp_t difference (duration)
 * @param msec  number of milliseconds
 * @return      the stamp difference
 */
#define nabtoMsec2Stamp(msec)   ((nabto_stamp_t)(msec * TICK_MILLISECOND))

#endif 
