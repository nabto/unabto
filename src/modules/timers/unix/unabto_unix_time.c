/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#include "unabto_unix_time.h"
#include <unabto/unabto_memory.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

void unabto_unix_timer_add_stamp(nabto_stamp_t* stamp, int msec) {
    stamp->milliseconds += msec;
}

static NABTO_THREAD_LOCAL_STORAGE bool auto_update_enabled = true;
static NABTO_THREAD_LOCAL_STORAGE struct unabto_unix_time cachedTime;

void unabto_time_auto_update(bool enabled) {
    auto_update_enabled = enabled;
}

void unabto_time_update_stamp() {
    uint64_t tmp64;
    uint64_t milliseconds;

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

    // use temporary 64 bit var to make overflow prevention explicit (vs 1000ll notation)
    tmp64 = res.tv_sec;
    tmp64 *= 1000;
    milliseconds = tmp64;

    tmp64 = res.tv_nsec;
    tmp64 /= 1000000;
    milliseconds += tmp64;

    if (cachedTime.milliseconds != milliseconds) {
        cachedTime.milliseconds = milliseconds;
        cachedTime.count = 0;
    }
}


struct unabto_unix_time unabto_unix_timer_get_stamp() {
    if (auto_update_enabled) {
        unabto_time_update_stamp();
    }
    cachedTime.count++;
    return cachedTime;
}

// return s1 < s2
bool unabto_unix_timer_stamp_less(nabto_stamp_t* s1, nabto_stamp_t* s2) {
    if (s1->milliseconds < s2->milliseconds) {
        return true;
    }
    if (s1->milliseconds == s2->milliseconds && s1->count < s2->count) {
        return true;
    }
    return false;
}

bool unabto_unix_timer_stamp_less_equal(nabto_stamp_t* s1, nabto_stamp_t* s2) {
    if (s1->milliseconds < s2->milliseconds) {
        return true;
    }
    if (s1->milliseconds == s2->milliseconds && s1->count <= s2->count) {
        return true;
    }
    return false;
}


bool unabto_unix_timer_is_stamppassed(nabto_stamp_t* stamp) {
    nabto_stamp_t now = nabtoGetStamp();
    return unabto_unix_timer_stamp_less_equal(stamp, &now);
}

nabto_stamp_diff_t unabto_unix_timer_stampdiff(nabto_stamp_t * newest, nabto_stamp_t * oldest) {
    return newest->milliseconds - oldest->milliseconds;
}

int unabto_unix_timer_stampdiff2ms(nabto_stamp_diff_t diff) {
    return (int) (diff);
}
