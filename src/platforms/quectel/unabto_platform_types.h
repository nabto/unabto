/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_PLATFORM_TYPES_H_
#define _UNABTO_PLATFORM_TYPES_H_

#define NABTO_INVALID_SOCKET    NULL
#define nabtoMsec2Stamp

typedef u8 uint8_t;
typedef s8 int8_t;
typedef u16 uint16_t;
typedef s16 int16_t;
typedef u32 uint32_t;
typedef s32 int32_t;
typedef u64 ssize_t;
typedef u8 nabto_socket_t;
typedef u64 nabto_stamp_t;
typedef u64 nabto_stamp_diff_t;

enum
{
  false,
  true
};
typedef uint8_t bool;


#endif
