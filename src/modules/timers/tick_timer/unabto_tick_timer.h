/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_TICK_TIMER_H_
#define _UNABTO_TICK_TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This is a ms based tick timer implementation of the timing
 * functionality needed by uNabto.
 */

//#include "../../../unabto_env_base.h"

typedef int32_t nabto_stamp_t;
typedef int32_t nabto_stamp_diff_t;

#define nabtoMsec2Stamp(x) (x)

void unabto_tick_timer_tick(int32_t ms);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
