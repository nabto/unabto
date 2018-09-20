/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_ENV_BASE_EVOLUTION_
#define _UNABTO_ENV_BASE_EVOLUTION_
/**
 * @file
 * The basic environment for uNabto Evolution SDK - Interface.
 */

#ifndef _DEFINED_SDK_TYPES_
////////////////////////////////
//BEGIN ColdFire/Lantronix FIX
//The next is taken from the Lantronix SDK include/coldfire/evolution_types.h
//Unfortunately include/coldfire/evolution_types.h can't be included because it defines
//ptrdiff_t differently than the Sourcery CodeBench for ColdFire
#define _DEFINED_SDK_TYPES_

/*!
 * ** \brief Type modifier to force pointers to be near.
 * **
 * ** This function is provided for compatibility with platforms with a
 * ** segmented architecture.
 * */
#define NEAR

/*!
 * ** \brief Type modifier to force pointers to be far.
 * **
 * ** This function is provided for compatibility with platforms with a
 * ** segmented architecture.
 * */
#define FAR 

/*!
 * ** \brief Type modifier to force pointers to be huge.
 * **
 * ** This function is provided for compatibility with platforms with a
 * ** segmented architecture.
 * */
#define HUGE 


/** Unsigned 8 bit quantity */
typedef unsigned char uint8_t;

/** Unsigned 16 bit quantity */
typedef unsigned short uint16_t;

/** Unsigned 32 bit quantity */
typedef unsigned long uint32_t;

/** Unsigned 64 bit quantity */
typedef unsigned long long uint64_t; /* NOT AVAILABLE ON ALL PLATFORMS */
  
/** Signed 8 bit quantity */
typedef signed char sint8_t;

/** Signed 16 bit quantity */
typedef signed short sint16_t;

/** Signed 32 bit quantity */
typedef signed long sint32_t;

/** Signed 64 bit quantity */
typedef signed long long sint64_t; /* NOT AVAILABLE ON ALL PLATFORMS */

/** Boolean data type */
typedef unsigned char bool;

/*!
 ** \brief Boolean true.
 **
 ** Use this to indicate a true condition or result.
 */
#define true 1          

/*!
 ** \brief Boolean false.
 **
 ** Use this to indicate a false condition or result.
 */
#define false 0

//END ColdFire/Lantronix FIX
////////////////////////////////
#endif

#include <evolution.h>

#ifndef _SYS_TYPES_H
typedef long int ssize_t;
#endif
//typedef uint8_t nabto_socket_t;     ///< Handle for UDP stack
typedef struct nabto_socket_t nabto_socket_t;

enum nabto_socket_type {
    NABTO_SOCKET_IP_V4,
    NABTO_SOCKET_IP_V6
};

struct nabto_socket_t {
    uint8_t sock;
    enum nabto_socket_type type;
};
typedef long long nabto_stamp_t;    ///< Nabto time stamp

#define NABTO_INVALID_SOCKET    -1
#define nabtoMsec2Stamp(msec)   (msec)
#define NABTO_FATAL_EXIT        KillThread(0);

#endif
