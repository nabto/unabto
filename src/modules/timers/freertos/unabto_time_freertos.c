/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#include "unabto_time_freertos.h"
#include "unabto/unabto_env_base.h"

nabto_stamp_t nabtoGetStamp() {
    return xTaskGetTickCount();
}

#define MAX_STAMP_DIFF 0x7fffffff;

bool nabtoIsStampPassed(nabto_stamp_t *stamp) {
    return *(uint32_t*)stamp - (uint32_t)nabtoGetStamp() > MAX_STAMP_DIFF;
}

nabto_stamp_diff_t nabtoStampDiff(nabto_stamp_t * newest, nabto_stamp_t * oldest) {
    return (*newest - *oldest);
}

int nabtoStampDiff2ms(nabto_stamp_diff_t diff) {
    return (int) diff*portTICK_PERIOD_MS;
}
