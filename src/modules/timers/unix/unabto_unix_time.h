/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_UNIX_TIMER_H_
#define _UNABTO_UNIX_TIMER_H_

#include <stdbool.h>

#include "unabto_unix_time_types.h"

#ifdef __cplusplus
extern "C" {
#endif


nabto_stamp_t unabto_unix_timer_get_stamp();
void unabto_unix_timer_add_stamp(nabto_stamp_t* stamp, int msec);
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

#define nabtoGetStamp unabto_unix_timer_get_stamp

#define nabtoMsec2Stamp(msec)   (msec)

#define nabtoAddStamp(stamp, msec) unabto_unix_timer_add_stamp(stamp, msec)
#define nabtoStampLess(s1, s2) unabto_unix_timer_stamp_less(s1, s2)
#define nabtoStampLessOrEqual(s1, s2) unabto_unix_timer_stamp_less_equal(s1, s2)


#define nabtoStampDiff2ms(diff) unabto_unix_timer_stampdiff2ms(diff)
#define nabtoStampDiff(newest, oldest) unabto_unix_timer_stampdiff(newest, oldest)
#define nabtoIsStampPassed(stamp) unabto_unix_timer_is_stamppassed(stamp)


#ifdef __cplusplus
} //extern "C"
#endif

#endif
