/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
/**
 * @file
 * The uNabto ServerPeer, common utility parts.
 */
 
#ifndef _UNABTO_UTIL_H_
#define _UNABTO_UTIL_H_

#if NABTO_SLIM
#include <unabto_platform_types.h>
#else
#include <unabto/unabto_env_base.h>
#endif

/******************************************************************************/
/* Utilities for safe access to possibly non-aligned multi-byte integers *
 * May be simplified on many platforms
 * Default behavior can be overridden by guarding the macros 
 * with #ifndef's
 */

#ifndef READ

/**
 * Read data from a buffer and copy it to the destination
 * @param dst the destination
 * @param src the source
 * @param size number of bytes to copy
 * @return
 */
#define READ(dst, src, size) do { size_t i; for(i = 0; i<size; i++) { dst[i] = src[i]; } } while (0)
#endif

#ifndef READ_U8
/**
 * Read an unsigned 1-byte integer.
 * @param u8   the integer.
 * @param src  source address
 * @return
 */
#define READ_U8(u8, src)        do { u8 = *((uint8_t*)(src)); } while (0)
#endif

#ifndef WRITE_U8
/**
 * Write an unsigned 1-byte integer.
 * @param dst  destination address.
 * @param u8   the integer
 * @return
 */
#define WRITE_U8(dst, u8)       do { *((uint8_t*)(dst)) = u8; } while (0)
#endif

#ifndef READ_U16
/**
 * Read an unsigned 2-byte integer.
 * @param u16  the integer.
 * @param src  source address
 * @return
 */
#define READ_U16(u16, src) do {           \
    uint8_t* src8 = (uint8_t*)(src);      \
    (u16) = (((uint16_t)src8[0]) << 8) |  \
            ( (uint16_t)src8[1]      );   \
} while (0)
#endif

#ifndef WRITE_U16
/**
 * Write an unsigned 2-byte integer.
 * @param dst  destination address.
 * @param u16  the integer
 * @return
 */
#define WRITE_U16(dst, u16) do {               \
    uint8_t* dst8 = (uint8_t*)(dst);           \
    dst8[0] = (uint8_t)(((uint16_t)(u16) >> 8) & 0xff); \
    dst8[1] = (uint8_t)( (uint16_t)(u16)       & 0xff);  \
} while (0);
#endif

#ifndef READ_U32
/**
 * Read an unsigned 4-byte integer in network byte order.
 * @param u32  the integer.
 * @param src  source address
 * @return
 */
#define READ_U32(u32, src) do {            \
    uint8_t* src8 = (uint8_t*)(src);       \
    (u32) = (((uint32_t)src8[0]) << 24) |  \
            (((uint32_t)src8[1]) << 16) |  \
            (((uint32_t)src8[2]) <<  8) |  \
            ( (uint32_t)src8[3]       );   \
} while (0)
#endif

#ifndef WRITE_U32
/**
 * Write an unsigned 4-byte integer to network byte order.
 * @param dst  destination address.
 * @param u32  the integer
 * @return
 */
#define WRITE_U32(dst, u32) do {                \
    uint8_t* dst8 = (uint8_t*)(dst);            \
    dst8[0] = (uint8_t)(((uint32_t)(u32) >> 24) & 0xff);  \
    dst8[1] = (uint8_t)(((uint32_t)(u32) >> 16) & 0xff);  \
    dst8[2] = (uint8_t)(((uint32_t)(u32) >>  8) & 0xff);  \
    dst8[3] = (uint8_t)( (uint32_t)(u32)        & 0xff);  \
} while (0);     
#endif

#ifndef WRITE_32
/**
 * Write a signed 4-byte integer.
 * @param dst  destination address.
 * @param s32  the integer
 * @return
 */
#define WRITE_32(dst, s32)      WRITE_U32(dst, (uint32_t)(s32))
#endif

// Macros for performing sequential read/write operations on a buffer.
#define READ_FORWARD(dst, pointer, size) do { READ(dst, pointer, size); pointer += size; } while(0)
#define READ_FORWARD_U8(value, pointer) do { READ_U8(value, pointer); pointer += 1; } while(0)
#define READ_FORWARD_U16(value, pointer) do { READ_U16(value, pointer); pointer += 2; } while(0)
#define READ_FORWARD_U32(value, pointer) do { READ_U32(value, pointer); pointer += 4; } while(0)
#define WRITE_FORWARD_U8(pointer, value) do { WRITE_U8(pointer, value); pointer += 1; } while (0)
#define WRITE_FORWARD_U16(pointer, value) do { WRITE_U16(pointer, value); pointer += 2; } while (0)
#define WRITE_FORWARD_U32(pointer, value) do { WRITE_U32(pointer, value); pointer += 4; } while (0)

/** @return max of two values. @param x first value @param y second value */
#define MAX(x, y)                   (((x) > (y)) ? (x) : (y))
#define MAX3(a, b, c)               MAX(MAX(a, b), c)
#define MAX4(a, b, c, d)            MAX(MAX(MAX(a, b), c), d)

/** @return min of two values. @param x first value @param y second value */
#define MIN(x, y)                   (((x) < (y)) ? (x) : (y))
#define MIN3(a, b, c)               MIN(MIN(a, b), c)
#define MIN4(a, b, c, d)            MIN(MIN(MIN(a, b), c), d)

#endif
