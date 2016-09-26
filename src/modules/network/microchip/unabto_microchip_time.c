/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#include <unabto/unabto_env_base.h>

nabto_stamp_t nabtoGetStamp(void)
{
    return TickGet();
}

/**
 * The stamp is a finite number so wrap arounds will happen
 * this means a simple equality check isn't enough.
 * A simple solution is to use the difference between unsigned stamps.
 */

#define MAX_STAMP_DIFF 0x7fffffff;

bool nabtoIsStampPassed(nabto_stamp_t *stamp) {
    return *stamp - nabtoGetStamp() > MAX_STAMP_DIFF;
}


nabto_stamp_diff_t nabtoStampDiff(nabto_stamp_t * newest, nabto_stamp_t * oldest) {
    return (*newest - *oldest);
}


int nabtoStampDiff2ms(nabto_stamp_diff_t tickDiff) {
    // TICK_MILLISECONDS have unit ticks/ms.
    // tickDiff have unit ticks.
    // ticks / TICK_MILLISECOND = ticks / (ticks/ms) = ticks * (ms/ticks) = ms
    return (int) tickDiff / TICK_MILLISECOND;
}

