/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#ifndef _DEBUG_CHANNEL_H_
#define _DEBUG_CHANNEL_H_

#if NABTO_BUILDING_BOOTLOADER
#if __DEBUG__
#define NABTO_USE_DEBUG_CHANNEL 1
#endif
#include <platform/platform.h>
#else
#include <unabto/unabto_env_base.h>
#endif

#if NABTO_USE_DEBUG_CHANNEL

void debug_channel_initialize(void);
void debug_channel_uninitialize(void);
void debug_channel_write(uint8_t value);
void debug_channel_write_string(const char* string);
void debug_channel_write_string_pgm(const far rom char* string);
void debug_channel_write_uint8_hex(uint8_t value);
void debug_channel_write_uint16_hex(uint16_t value);
void debug_channel_write_int32(int32_t value);

#else

#define debug_channel_initialize()
#define debug_channel_uninitialize()
#define debug_channel_write(value)
#define debug_channel_write_string(string)
#define debug_channel_write_string_pgm(string)
#define debug_channel_write_uint8_hex(value)
#define debug_channel_write_uint16_hex(value)
#define debug_channel_write_int32(value)

#endif

#endif
