/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#include "unabto_env_base.h"

#include "FreeRTOS.h"
#include "task.h"

nabto_stamp_t nabtoGetStamp()
{
    return  xTaskGetTickCount();
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
