/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_PLATFORM_TYPES_H_
#define _UNABTO_PLATFORM_TYPES_H_

#if (defined(WIN32) && _MSC_VER < 1800) || defined(WINCE)
typedef __int64     int64_t;
typedef int         int32_t;
typedef short int   int16_t;
typedef signed char int8_t;
typedef unsigned _int64 uint64_t;
typedef unsigned int    uint32_t;
typedef unsigned short  uint16_t;
typedef unsigned char   uint8_t;
#else
#include <stdint.h>
#endif

// size_t
#include <stddef.h>

#if defined(__cplusplus) && _MSC_VER >= 1800

#include <stdbool.h>

#else

/** for convenience and portability */
enum { false, true };

/** for convenience and portability */
typedef unsigned char bool;

#endif

#define PRIptrdiff "i"
#define PRIsize "i"

#include <platforms/unabto_common_types.h>

#include <basetsd.h> // SSIZE_T

/** for convenience and portability */
//typedef unsigned long  uint32_t;

/** for convenience and portability */
//typedef signed long  int32_t;

/** for convenience and portability */
typedef unsigned short uint16_t;

/** for convenience and portability */
//typedef unsigned short int16_t;

/** for convenience and portability */
typedef unsigned char uint8_t;

/** for convenience and portability */
//typedef unsigned char int8_t;

/** for convenience and portability */
typedef SSIZE_T ssize_t;

#endif
