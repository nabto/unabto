/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_PRINTF_LOGGER_H_
#define _UNABTO_PRINTF_LOGGER_H_

#include <unabto/unabto_env_base.h>

#ifdef __cplusplus
extern "C" {
#endif

#if NABTO_ENABLE_LOGGING

void log_buffer(const uint8_t* buf, size_t buflen);

#if UNABTO_PLATFORM_PIC18
void log_buffer_pgm(const __ROM void* buffer, uint24_t length);
#endif

#else

#define log_buffer(buf, buflen)

#if UNABTO_PLATFORM_PIC18
#define log_buffer_pgm(buffer, length)
#endif

#endif


#ifdef __cplusplus
} //extern "C"
#endif

#endif
