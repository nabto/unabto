/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#include <stdio.h>
#include <string.h>

#include <modules/log/unabto_basename.h>
#include <unabto_logging_defines.h>
#include "unabto_logging_android.h"

#define ANDROID_LOG_BUFFER_SIZE 256

char log_buffer_temp[ANDROID_LOG_BUFFER_SIZE];
uint16_t log_buffer_level;

uint16_t unabto_android_map_log_level(uint16_t loglevel) {
    switch (loglevel) {
        case NABTO_LOG_SEVERITY_FATAL:
            return ANDROID_LOG_FATAL;
        case NABTO_LOG_SEVERITY_ERROR:
            return ANDROID_LOG_ERROR;
        case NABTO_LOG_SEVERITY_WARN:
            return ANDROID_LOG_WARN;
        case NABTO_LOG_SEVERITY_INFO:
            return ANDROID_LOG_INFO;
        case NABTO_LOG_SEVERITY_DEBUG:
            return ANDROID_LOG_DEBUG;
        default: return ANDROID_LOG_VERBOSE;
    }
}

void unabto_android_log_start(uint16_t loglevel, const char *file, unsigned int line) {
    log_buffer_level = unabto_android_map_log_level(loglevel);
    snprintf(log_buffer_temp, ANDROID_LOG_BUFFER_SIZE, "%s(%u):", unabto_basename(file), (line));
}

void unabto_android_log_end(const char* format, ...) {
    char buffer[ANDROID_LOG_BUFFER_SIZE];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, ANDROID_LOG_BUFFER_SIZE, format, args);
    va_end(args);

    __android_log_print(log_buffer_level, UNABTO_TAG, "%s %s", log_buffer_temp, buffer);
}

void log_buffer(const uint8_t *buf, size_t buflen) {
    size_t i, j;
    size_t linenums = (buflen + 15) / 16;

    for (i = 0; i < linenums; i++) {
        size_t len = 0;
        memset(log_buffer_temp, 0, sizeof(log_buffer_temp));

        len += snprintf(log_buffer_temp + len, ANDROID_LOG_BUFFER_SIZE, "%02x  ", (unsigned int)i * 16);
        for (j = 0; j < 16 && i * 16 + j < buflen; j++) {
            len += snprintf(log_buffer_temp + len, ANDROID_LOG_BUFFER_SIZE, "%02x", buf[i * 16 + j]);
            if (j < 15) {
                len += snprintf(log_buffer_temp + len, ANDROID_LOG_BUFFER_SIZE, " ");
            }
        }
        __android_log_print(ANDROID_LOG_INFO, UNABTO_TAG, "%s", log_buffer_temp);
    }
}
