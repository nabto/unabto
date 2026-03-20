/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#include <unabto/unabto_external_environment.h>

#include <openssl/rand.h>

void nabto_random(uint8_t* buf, size_t len) {
    if (!RAND_bytes((unsigned char*)buf, (int)len)) {
        NABTO_LOG_FATAL(("RAND_bytes failed."));
    }
}
