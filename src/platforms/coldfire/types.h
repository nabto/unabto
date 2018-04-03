/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _TYPES_H_
#define _TYPES_H_

#include <stddef.h>
#include <null.h>
#include <stdint.h>

typedef int bool;
enum {
    false,
    true
};

// Someone please make this macro! Looks like this on PIC18 (little endian - 8 bit)
// #define MAKE_IP_PRINTABLE(ip) (uint8_t)(ip >> 24), (uint8_t)(ip >> 16), (uint8_t)(ip >> 8), (uint8_t)(ip)
#define MAKE_IP_PRINTABLE(ip) !!!!


#endif
