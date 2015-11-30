/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */

#ifndef _UNABTO_PLATFORM_LOGGING_H_
#define _UNABTO_PLATFORM_LOGGING_H_

#if NABTO_ENABLE_LOGGING

#include <stdio.h>

#define NABTO_LOG_BASIC_PRINT(level, message) do { printf message; printf("\n"); } while(0)

#else

#define NABTO_LOG_BASIC_PRINT(level, message) do { } while(0)

#endif

#endif
