/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_PLATFORM_TYPES_H_
#define _UNABTO_PLATFORM_TYPES_H_

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <platforms/unabto_common_types.h>

typedef long ssize_t;
typedef uint8_t nabto_socket_t;
typedef long long nabto_stamp_t;
typedef long long nabto_stamp_diff_t;

#define NABTO_INVALID_SOCKET    NULL
#define nabtoMsec2Stamp(msec)   (msec)
#define NABTO_FATAL_EXIT        while(1);

#endif
