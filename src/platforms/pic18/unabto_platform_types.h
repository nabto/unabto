/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_PLATFORM_TYPES_H_
#define _UNABTO_PLATFORM_TYPES_H_

#include <p18cxxx.h>
#include <GenericTypeDefs.h>

#define __ROM far rom const
#define __USE_ROM 1

typedef UINT8 uint8_t;
typedef INT8 int8_t;
typedef UINT16 uint16_t;
typedef INT16 int16_t;
typedef UINT24 uint24_t;
typedef UINT32 uint32_t;
typedef INT32 int32_t;
typedef WORD ssize_t;

enum
{
  false,
  true
};
typedef uint8_t bool;

#define __LENGTH_MODIFIER_8                                                             "hh"
#define __LENGTH_MODIFIER_16                                                            "h"
#define __LENGTH_MODIFIER_32                                                            "l"
#define __LENGTH_MODIFIER_64                                                            "ll"

#define PRItext                                                                         "HS"

#define MAKE_IP_PRINTABLE(ip) (uint8_t)(ip >> 24), (uint8_t)(ip >> 16), (uint8_t)(ip >> 8), (uint8_t)(ip)

#include <../unabto_common_types.h>

#endif
