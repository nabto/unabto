/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_ENV_BASE_ARDUINO_H_
#define _UNABTO_ENV_BASE_ARDUINO_H_

#include <unabto/unabto_util.h>

#include <stdint.h>
#include <stdbool.h>
#include <platforms/unabto_common_types.h>

/**
 * Generic typedefs
 */

typedef int ssize_t;

/**
 * Socket related definitions
 */

#define 	htons(s)   ((s<<8) | (s>>8))
#define 	htonl(l)   ((l<<24) | ((l&0x00FF0000l)>>8) | ((l&0x0000FF00l)<<8) | (l>>24))
#define     ntohl htonl
#define     ntohs htons

/**
 * Time related definitions
 */

typedef unsigned long int nabto_stamp_t;
typedef long int          nabto_stamp_diff_t;

#define nabtoGetStamp(void) millis()

#define nabtoMsec2Stamp

/**
 * Logging related definitions
 */
 
#include <modules/network/w5100/w5100_network.h>

#if NABTO_ENABLE_LOGGING
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

void p(char *fmt, ... );

#ifdef __cplusplus
} //extern "C"
#endif

#define NABTO_LOG_INFO(cmsg) if (0) {} else { p cmsg; p("\n"); }
#define NABTO_LOG_BASIC_PRINT(Severity,msg) p msg
#endif

#endif
