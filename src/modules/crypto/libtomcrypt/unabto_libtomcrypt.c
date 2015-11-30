/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#include <unabto/unabto_aes_cbc.h>
#include <unabto/unabto_hmac_sha256.h>
#include <unabto/util/unabto_buffer.h>
#include "tomcrypt.h"

bool aes128_cbc_encrypt(const uint8_t* key, uint8_t* input, uint16_t input_len) {
    /**
     * first 16 bytes of the input is the iv.
     */
    symmetric_CBC cbc_ctx;
    int cipher;
    int res;

    register_cipher(&aes_desc);
    cipher = find_cipher("aes");
    if (cipher < 0) {
        NABTO_LOG_FATAL(("Can't find cipher aes"));
    }

    res = cbc_start(cipher, input /* input[0..15] is iv */, key, 16 /*keylength in octets*/, 10 /* 10 rounds for aes128 */, &cbc_ctx);

    if (res == CRYPT_OK) {
        res = cbc_encrypt(input+16, input+16, input_len - 16, &cbc_ctx);
    }

    if (res == CRYPT_OK && cbc_done(&cbc_ctx) == CRYPT_OK) {
        return true;
    }
    return false;
}

bool aes128_cbc_decrypt(const uint8_t* key, uint8_t* input, uint16_t input_len) {

    symmetric_CBC cbc_ctx;
    int cipher;
    int res;

    register_cipher(&aes_desc);
    cipher = find_cipher("aes");
    if (cipher < 0) {
        NABTO_LOG_FATAL(("Can't find cipher aes"));
    }
    
    res = cbc_start(cipher, input /* input[0..15] is iv */, key, 16 /*keylength in octets*/, 10 /* 10 rounds for aes128 */, &cbc_ctx);
    if (res == CRYPT_OK) {
        cbc_decrypt(input+16, input+16, input_len - 16, &cbc_ctx);
    }
    
    if (res == CRYPT_OK && cbc_done(&cbc_ctx) == CRYPT_OK) {
        return true;
    }
    return false;
}

void unabto_hmac_sha256_buffers(const buffer_t keys[], uint8_t keysSize,
                                const buffer_t messages[], uint8_t messagesSize,
                                uint8_t *mac, uint16_t macSize) 
{

    
    uint16_t keySize = 0;
    uint16_t i;

    uint8_t key[UNABTO_SHA256_BLOCK_LENGTH];
    uint8_t* ptr;
    int res;
    int hash = 0;
    hmac_state hmac;
    unsigned long outlen;

    for (i = 0; i < keysSize; i++) {
        keySize += keys[i].size;
    }

    UNABTO_ASSERT(keySize <= UNABTO_SHA256_BLOCK_LENGTH);

    ptr = key;
    for(i = 0; i < keysSize; i++) {
        memcpy(ptr, (const void*) keys[i].data, keys[i].size);
        ptr += keys[i].size;
    }
    
    register_hash(&sha256_desc);
    
    if (find_hash("sha256") < 0) {
        NABTO_LOG_FATAL(("can't find hash"));
        return;
    }
    
    res = hmac_init(&hmac, hash, key, keySize);
    if (res != CRYPT_OK) {
        NABTO_LOG_FATAL(("This should not fail"));
    }
    
    for (i = 0; i < messagesSize; i++) {
        if (messages[i].size > 0) {
            res = hmac_process(&hmac, messages[i].data, messages[i].size);
            if (res != CRYPT_OK) {
                NABTO_LOG_FATAL(("This should not fail"));
            }
        }
    }
    outlen = macSize;
    res = hmac_done(&hmac, mac, &outlen);
    if (res != CRYPT_OK) {
        NABTO_LOG_FATAL(("This should not fail"));
    }
}
