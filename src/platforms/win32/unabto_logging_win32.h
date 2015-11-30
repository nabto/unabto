/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_LOGGING_WIN32_H_
#define _UNABTO_LOGGING_WIN32_H_
/**
 * @file
 * The environment for the Nabto Micro Device Server (PC Device), Interface.
 *
 * This file holds various logging declarations.
 */

#include <modules/log/unabto_log_header.h>

#include <stdio.h>

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PRINT_ENDPOINT 1   /**< enable printing of endpoints */

#ifndef NABTO_LOG_BASIC_PRINT
/** Print debugging info.
 * @param loglevel  logging level
 * @param cmsg      the message
 */
#define NABTO_LOG_BASIC_PRINT(loglevel, cmsg) do {    \
    unabto_log_header(__FILE__, __LINE__);             \
    printf cmsg;                                      \
    printf("\n");                                     \
    fflush(stdout);                                   \
} while(0)
#endif

#ifdef __cplusplus
} //extern "C"
#endif

#endif
