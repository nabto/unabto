/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */

#ifndef _UNABTO_PLATFORM_LOGGING_H_
#define _UNABTO_PLATFORM_LOGGING_H_

#define NABTO_FATAL_EXIT do { } while(1)

#if NABTO_ENABLE_LOGGING

#include <stdio.h>
#include <os.h>

const char* clean_filename(const char* path);

extern OS_SEM loggingSemaphore;

//#define NABTO_LOG_BASIC_PRINT(severity, message) do { OS_ERR osErr; printf("\n%10" PRIu32 " %" PRItext ":%u  ", OSTimeGet(&osErr), clean_filename(__FILE__), __LINE__); printf message; } while(0)
#define NABTO_LOG_BASIC_PRINT(severity, message) do { OS_ERR osErr; OSSemPend(&loggingSemaphore, 0, OS_OPT_PEND_BLOCKING, NULL, &osErr); printf("\n%10" PRIu32 " %" PRItext ":%u  ", OSTimeGet(&osErr), clean_filename(__FILE__), __LINE__); printf message; OSSemPost(&loggingSemaphore, OS_OPT_POST_NO_SCHED, &osErr); } while(0)

#else

#define NABTO_LOG_BASIC_PRINT(severity, message) do { } while(0)

#endif

#endif
