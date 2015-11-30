/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_LOGGING_UNIX_H_
#define _UNABTO_LOGGING_UNIX_H_
/**
 * @file
 * Log implementation for unix like platforms.
 */

#include <stdio.h>

#include <modules/log/unabto_log_header.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PRINT_ENDPOINT 1   /**< enable printing of endpoints */

/** Print debugging info.
 * @param loglevel  logging level
 * @param cmsg      the message
 */
#ifndef NABTO_LOG_BASIC_PRINT
#define NABTO_LOG_BASIC_PRINT(loglevel, cmsg) \
do {                                          \
    unabto_log_header(__FILE__, __LINE__);       \
    printf cmsg;                              \
    printf("\n");                             \
} while(0)
#endif


#ifdef __cplusplus
} //extern "C"
#endif

#endif
