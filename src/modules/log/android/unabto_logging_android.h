/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_LOGGING_ANDROID_H_
#define _UNABTO_LOGGING_ANDROID_H_

#include <jni.h>
#include <sys/types.h>
#include <android/log.h>

#define UNABTO_TAG "UNABTO"

void log_buffer(const uint8_t* buf, size_t buflen);
void unabto_android_log_start(uint16_t loglevel, const char* file, unsigned int line);
void unabto_android_log_end(const char* format, ...);

#define NABTO_LOG_BASIC_PRINT(loglevel, cmsg)               \
do {                                                        \
    unabto_android_log_start(loglevel, __FILE__, __LINE__); \
    unabto_android_log_end cmsg;                            \
} while(0)

#endif
