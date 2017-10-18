/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_ENCRYPTION

#include "unabto_env_base.h"

#if NABTO_ENABLE_UCRYPTO

#include "unabto_aes128_sha256.h"
#include "unabto_hmac_sha256.h"

#include <string.h>

bool unabto_truncated_hmac_sha256_verify_integrity(
    const uint8_t *key, uint16_t keyLength,
    const uint8_t *buf, uint16_t bufLength,
    const uint8_t integrity[16])
{
    uint8_t hmac[TRUNCATED_HMAC_SHA256_LENGTH];

    unabto_buffer keys[1];
    unabto_buffer messages[1];

    unabto_buffer_init(keys, (uint8_t*)key, keyLength);
    unabto_buffer_init(messages, (uint8_t*)buf, bufLength);


    unabto_hmac_sha256_buffers(keys, 1,
                               messages, 1,
                               hmac, TRUNCATED_HMAC_SHA256_LENGTH); //sizeof(hmac));

    return memcmp((const uint8_t*)hmac, integrity, TRUNCATED_HMAC_SHA256_LENGTH) == 0;
}

#endif
