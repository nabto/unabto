/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_PLATFORM_TYPES_H_
#define _UNABTO_PLATFORM_TYPES_H_

#include <stdint.h>
#include <stdbool.h>

//#define __ROM _Pragma("location=\"FLASH\"")
//#define __USE_ROM 1

//typedef unsigned int size_t;
typedef int16_t ssize_t;

#define PRI_IP "%u.%u.%u.%u"
#define PRI_IP_FORMAT(address) (uint8_t)(address>>24), (uint8_t)(address>>16), (uint8_t)(address>>8), (uint8_t)(address)

#include <platforms/unabto_common_types.h>

#endif
