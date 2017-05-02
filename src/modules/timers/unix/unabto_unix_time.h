/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_UNIX_TIMER_H_
#define _UNABTO_UNIX_TIMER_H_

#include <stdbool.h>

#include "unabto_unix_time_types.h"

#ifdef __cplusplus
extern "C" {
#endif


void unabto_unix_timer_add_stamp(nabto_stamp_t* stamp, int msec);
void unabto_unix_timer_add_stamp_ns(nabto_stamp_t* stamp, int nsec);
bool unabto_unix_timer_stamp_less(nabto_stamp_t* s1, nabto_stamp_t* s2);
bool unabto_unix_timer_stamp_less_equal(nabto_stamp_t* s1, nabto_stamp_t* s2);

/**
 * Switch auto update of the timestamp on/off
 * 
 * If switched off you need to call unabto_unix_time_update_stamp by
 * yourself from your application.
 */
void unabto_unix_time_auto_update(bool enabled);
void unabto_unix_time_update_stamp();

nabto_stamp_t nabtoGetStamp();

#define nabtoMsec2Stamp(msec)   (msec)

#define nabtoAddStamp(stamp, msec) unabto_unix_timer_add_stamp(stamp, msec)
#define nabtoAddStampNs(stamp, nsec) unabto_unix_timer_add_stamp_ns(stamp, nsec)
#define nabtoStampLess(s1, s2) unabto_unix_timer_stamp_less(s1, s2)
#define nabtoStampLessOrEqual(s1, s2) unabto_unix_timer_stamp_less_equal(s1, s2)

//#define nabtoSetFutureStampNs(stamp, nsec) unabto_unix_timer_add_stamp_ns(stamp, nsec);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
