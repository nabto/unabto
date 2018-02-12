/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_ALL

#include <unabto/unabto_env_base.h>
#include "unabto_printf_logger.h"

#if NABTO_ENABLE_LOGGING

#if NABTO_LOG_CHECK(NABTO_LOG_SEVERITY_BUFFERS)

void log_buffer(const uint8_t* buf, size_t buflen) {
    size_t i, j;
    size_t linenums = (buflen + 15) / 16;
    for (i = 0; i < linenums; i++) {
        printf("%02x  ", (unsigned int) i * 16);
        for (j = 0; j < 16 && i * 16 + j < buflen; j++) {
            printf("%02x", buf[i * 16 + j]);
            if (j < 15) printf(" ");
        }
        printf("\n");
    }
}

#if UNABTO_PLATFORM_PIC18

#define LINE_WIDTH      64ul

void log_buffer_pgm(const far rom void* buffer, uint24_t length) {
    uint24_t index = 0;
    uint24_t line;
    uint24_t i;
    const far rom uint8_t* b = (const far rom uint8_t*)buffer;
    uint16_t numberOfLines = (length + (LINE_WIDTH - 1)) / LINE_WIDTH;

    for (line = 0; line < numberOfLines; line++) {
        printf("%05"PRIx32"  ", (uint32_t) index);

        for (i = 0; i < LINE_WIDTH && index < length; i++) {
            printf("%02"PRIx8, b[index]);
            if (i < (LINE_WIDTH - 1)) {
                printf(" ");
            }
            index++;
        }
        printf("\n");
    }
}

#endif

#else

#if !UNABTO_PLATFORM_PIC18
void log_buffer(const uint8_t* buf, size_t buflen)
{ }
#endif

//void log_buffer(const uint8_t* buf, size_t buflen)
//{ }
//
//#if UNABTO_PLATFORM_PIC18
//
//void log_buffer_pgm(const __ROM void* buffer, uint24_t length)
//{ }
//
//#endif

#endif

#endif //NABTO_ENABLE_LOGGING
