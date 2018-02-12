/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
/**
 * @file
 *
 * The Time Handling basics for win32, Implementation
 */

#include <unabto/unabto_env_base.h>
#include <modules/timers/auto_update/unabto_time_auto_update.h>

static nabto_stamp_t cachedTime;
static bool autoUpdateEnabled = true;

void unabto_time_auto_update(bool enabled) {
    autoUpdateEnabled = enabled;
}

#if defined(WINCE) && (_WIN32_WCE < 0x600)
void unabto_time_update_stamp() {
  SYSTEMTIME st;
  FILETIME ft;
  UINT64 time;
  GetSystemTime(&st);
  SystemTimeToFileTime(&st, &ft);
  // convert struct to uint64
  time = ft.dwHighDateTime;
  time <<= 32;
  time |= ft.dwLowDateTime;
  
  time /= 10000; // change resolution from 100 ns to 1 ms
  
  cachedTime = time;
}
#else
void unabto_time_update_stamp() {
    // the new way of getting time
    FILETIME ft;
    UINT64 time;

    GetSystemTimeAsFileTime(&ft);

    // convert struct to uint64
    time = ft.dwHighDateTime;
    time <<= 32;
    time |= ft.dwLowDateTime;

    time /= 10000; // change resolution from 100 ns to 1 ms

    cachedTime = time;
}
#endif

nabto_stamp_t nabtoGetStamp()
{
    if (autoUpdateEnabled) {
        unabto_time_update_stamp();
    }
    return cachedTime;
}

bool nabtoIsStampPassed(nabto_stamp_t *stamp)
{
    bool val = (nabtoGetStamp() >= (*stamp));
    return val;
}

nabto_stamp_diff_t nabtoStampDiff(nabto_stamp_t * newest, nabto_stamp_t * oldest)
{
    return (*newest - *oldest);
}

int nabtoStampDiff2ms(nabto_stamp_diff_t diff)
{
    return (int) diff;
}
