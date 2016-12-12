/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */

#include <unabto/unabto_env_base.h>

#if NABTO_ENABLE_UCRYPTO

#include <unabto/unabto_hmac_sha256.h>
#include "unabto_sha256.h"
#include <string.h>

/* HMAC-SHA-256 functions */

static uint8_t blockPad[SHA256_BLOCK_LENGTH];
static uint8_t keyTemp[SHA256_BLOCK_LENGTH];
static uint8_t digestTemp[SHA256_DIGEST_LENGTH];

void unabto_hmac_sha256_buffers(const unabto_buffer keys[], uint8_t keysSize, const unabto_buffer messages[], uint8_t messagesSize, uint8_t *mac, uint16_t macSize) {
    uint8_t fill = 0;
    uint8_t num;
    uint8_t i;
    uint8_t key_size = 0;
    uint8_t* ptr = keyTemp;

    for (i = 0; i < keysSize; i++) {
        key_size += keys[i].size;
    }

    // Assume key_size <= SHA256_BLOCK_LENGTH
    for (i = 0; i < keysSize; i++) {
        memcpy(ptr, (const void*) keys[i].data, keys[i].size);
        ptr += keys[i].size;
    }
    
    num = key_size;

    fill = SHA256_BLOCK_LENGTH - num;

    memset(blockPad + num, 0x36, fill);
    for (i = 0; i < num; i++) {
        blockPad[i] = keyTemp[i] ^ 0x36;
    }
    unabto_sha256_init();
    unabto_sha256_update(blockPad, SHA256_BLOCK_LENGTH);
    for (i = 0; i < messagesSize; i++) {
        unabto_sha256_update(messages[i].data, messages[i].size);
    }
    unabto_sha256_final(digestTemp);
    memset(blockPad + num, 0x5c, fill);
    for (i = 0; i < num; i++) {
        blockPad[i] = keyTemp[i] ^ 0x5c;
    }

    unabto_sha256_init();
    unabto_sha256_update(blockPad, SHA256_BLOCK_LENGTH);
    unabto_sha256_update(digestTemp, SHA256_DIGEST_LENGTH);
    unabto_sha256_final(digestTemp);

    memcpy(mac, (void*) digestTemp, macSize);
}

#endif
