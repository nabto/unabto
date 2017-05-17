/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#include "unabto_unix_time.h"
#include <unabto/unabto_memory.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

void unabto_unix_timer_add_stamp(nabto_stamp_t* stamp, int msec) {
    stamp->tv_sec += msec/1000;
    stamp->tv_nsec += (msec%1000)*1000000;
    stamp->tv_sec += stamp->tv_nsec/1000000000;
    stamp->tv_nsec %= 1000000000;
}

static NABTO_THREAD_LOCAL_STORAGE bool auto_update_enabled = true;
static NABTO_THREAD_LOCAL_STORAGE struct timespec cachedTime;

void unabto_time_auto_update(bool enabled) {
    auto_update_enabled = enabled;
}

void unabto_time_update_stamp() {
    struct timespec res;
// OS X does not have clock_gettime, use clock_get_time
#ifdef __MACH__
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    res.tv_sec = mts.tv_sec;
    res.tv_nsec = mts.tv_nsec;
#elif defined(CLOCK_MONOTONIC)
    clock_gettime(CLOCK_MONOTONIC, &res);
#else
    clock_gettime(CLOCK_REALTIME, &res);
#endif
    cachedTime = res;
}


struct timespec nabtoGetStamp() {
    if (auto_update_enabled) {
        unabto_time_update_stamp();
    }
    return cachedTime;
}

// return s1 < s2
bool unabto_unix_timer_stamp_less(nabto_stamp_t* s1, nabto_stamp_t* s2) {
    if (s2->tv_sec < s1->tv_sec) {
        return false;
    }
    
    // s1 <= s2 here, if s1.sec == s2.sec then look at the nanoseconds.
    return s1->tv_sec < s2->tv_sec || ((s1->tv_nsec/1000000) < (s2->tv_nsec/1000000));
}

bool unabto_unix_timer_stamp_less_equal(nabto_stamp_t* s1, nabto_stamp_t* s2) {
    if (s2->tv_sec < s1->tv_sec) {
        return false;
    }
    
    // s1 <= s2 here, if s1.sec == s2.sec then look at the nanoseconds which is converted to milliseconds since that is the smallest timeunit in unabto.
    return s1->tv_sec < s2->tv_sec || ((s1->tv_nsec/1000000) <= (s2->tv_nsec/1000000));
}


bool nabtoIsStampPassed(nabto_stamp_t* stamp) {
    nabto_stamp_t now = nabtoGetStamp();
    return unabto_unix_timer_stamp_less_equal(stamp, &now);
}

nabto_stamp_diff_t nabtoStampDiff(nabto_stamp_t * newest, nabto_stamp_t * oldest) {
    int sec = newest->tv_sec - oldest->tv_sec;
    long nsec = newest->tv_nsec - oldest->tv_nsec;
    return (sec*1000+nsec/1000000);
}

int nabtoStampDiff2ms(nabto_stamp_diff_t diff) {
    return (int) (diff);
}
