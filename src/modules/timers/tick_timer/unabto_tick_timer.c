/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#include "../../../unabto/unabto_env_base.h"

nabto_stamp_t unabto_tick_timer_ticks;

nabto_stamp_t nabtoGetStamp() {
    return unabto_tick_timer_ticks;
}

bool nabtoIsStampPassed(nabto_stamp_t* stamp) {
    return ((nabtoGetStamp() - (*stamp)) >= 0);
}

void unabto_tick_timer_tick(int32_t ms) {
    unabto_tick_timer_ticks+=ms;
}


nabto_stamp_diff_t nabtoStampDiff(nabto_stamp_t * newest, nabto_stamp_t *oldest) {
    return *newest - *oldest;
}


int nabtoStampDiff2ms(nabto_stamp_diff_t diff) {
    return diff;
}
