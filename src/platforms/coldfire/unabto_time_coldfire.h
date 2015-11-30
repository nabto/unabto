/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_TIME_COLDFIRE_H_
#define _UNABTO_TIME_COLDFIRE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * We measure time in ticks.
 */
typedef uint32_t nabto_stamp_t;
typedef int32_t  nabto_stamp_diff_t;

nabto_stamp_t nabtoGetStamp(void);

/**
 * Convert milleseconds to nabto_stamp_t difference (duration)
 * @param msec  number of milliseconds
 * @return      the stamp difference
 */
#define nabtoMsec2Stamp(msec)   ((nabto_stamp_t)(msec*(1000ul/*TICK_SECOND*//1000ul)))

/**
 * Remove thread/process from active queue (if relevant)
 * @param msec  the number of milliseconds before retrieving work.
 *
 * This functionality may only be needed in polling/pseudo-polling environments.
 */
void nabtoYield(int msec);


#ifdef __cplusplus
} //extern "C"
#endif

#endif 
