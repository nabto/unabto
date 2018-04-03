/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
/**
 * Time defs. for the uNabto FreeRTOS server
 * @author jbk@nabto.com
 * @since 2011-09-13
 */
#ifndef _UNABTO_TIME_CUSTOM_H_
#define _UNABTO_TIME_CUSTOM_H_

#ifdef __cplusplus
extern "C" {
#endif


typedef system_tick_t nabto_stamp_t;
void nabtoYield(int msec);

#define nabtoMsec2Stamp(msec) (msec)


#ifdef __cplusplus
} //extern "C"
#endif

#endif
