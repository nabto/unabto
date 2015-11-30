/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_ENV_BASE_RENESAS_RL78_H_
#define _UNABTO_ENV_BASE_RENESAS_RL78_H_
/**
 * @file
 * The basic environment for uNabto Renesas RL78 - Interface.
 */

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <platforms/unabto_common_types.h>

typedef long ssize_t;                           ///< ssize_t definition
typedef uint8_t nabto_socket_t;			///< Handle for UDP stack
typedef long long nabto_stamp_t;		///< Nabto time stamp
typedef long long nabto_stamp_diff_t;

#define NABTO_INVALID_SOCKET    NULL
#define nabtoMsec2Stamp(msec)   (msec)
#define NABTO_FATAL_EXIT        exit(1);


#endif
