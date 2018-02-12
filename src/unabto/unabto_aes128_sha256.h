/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_AES128_SHA256_H_
#define _UNABTO_AES128_SHA256_H_

#include <unabto/unabto_env_base.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    HMAC_SHA256_KEYLENGTH = 32,
    TRUNCATED_HMAC_SHA256_LENGTH = 16
};

bool unabto_truncated_hmac_sha256_verify_integrity(
    const uint8_t *key, uint16_t keyLength,
    const uint8_t *buf, uint16_t bufLength,
    const uint8_t integrity[16]);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
