/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#include "../../unabto_env_base.h"

nabto_stamp_t nabtoGetStamp(void)
{
    return 0;//TickGet();
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

int nabtoStampDiff2ms(nabto_stamp_diff_t diff) {
    return (int) diff;
}

