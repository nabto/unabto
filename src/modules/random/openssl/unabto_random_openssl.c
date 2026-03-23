/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#include <unabto/unabto_external_environment.h>

#include <openssl/rand.h>
#include <limits.h>

void nabto_random(uint8_t* buf, size_t len) {
    if (len >= INT_MAX) {
        NABTO_LOG_FATAL(("nabto_random called with len >= INT_MAX"));
        return;
    }
    if (!RAND_bytes((unsigned char*)buf, (int)len)) {
        NABTO_LOG_FATAL(("RAND_bytes failed."));
    }
}
