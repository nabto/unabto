/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_LOGGING_RENESAS_RL78_H_
#define _UNABTO_LOGGING_RENESAS_RL78_H_
/**
 * @file
 * Logging for uNabto Renesas RL78 - Interface.
 */

#if NABTO_ENABLE_LOGGING

#include <system\console.h>

//#define NABTO_LOG_BASIC_PRINT(severity, message) do { nabto_renesas_rl78_log(message); nabto_renesas_rl78_log("\n"); } while(0)
#define NABTO_LOG_BASIC_PRINT(severity, message) do { ConsolePrintf(message); ConsolePrintf("\n"); } while(0)

#ifdef __cplusplus
extern "C" {
#endif

void nabto_renesas_rl78_log(char *message);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NABTO_ENABLE_LOGGING */


#endif
