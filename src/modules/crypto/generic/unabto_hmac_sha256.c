/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_ENCRYPTION

#include <unabto/unabto_env_base.h>

#if NABTO_ENABLE_UCRYPTO

#include <unabto/unabto_hmac_sha256.h>
#include "unabto_sha256.h"

#include <string.h>

/* HMAC-SHA-256 functions */

static sha256_ctx sha_ctx;
static uint8_t block_pad[SHA256_BLOCK_LENGTH];

//void print_sha256_ctx(sha256_ctx* ctx);

void unabto_hmac_sha256_buffers(const unabto_buffer keys[], uint8_t keys_size,
                                const unabto_buffer messages[], uint8_t messages_size,
                                uint8_t *mac, uint16_t mac_size)
{
    uint16_t fill = 0;
    uint16_t num;
    const uint8_t *key_used;
    uint8_t i;
    uint8_t key_temp[SHA256_BLOCK_LENGTH];
    uint8_t digest_temp[SHA256_DIGEST_LENGTH];

    uint16_t key_size = 0;
    for (i = 0; i < keys_size; i++) {
        key_size += keys[i].size;
    }


    if (key_size <= SHA256_BLOCK_LENGTH) {
        uint8_t* ptr = key_temp;
        for (i = 0; i < keys_size; i++) {
            memcpy(ptr, (const void*)keys[i].data, keys[i].size);
            ptr += keys[i].size;
        }
        key_used = (const uint8_t*) key_temp;
        num = key_size;
    } else { // key_size > SHA256_BLOCK_LENGTH
        num = SHA256_DIGEST_LENGTH;
        //NABTO_LOG_TRACE(("key size %i", key_size));
        //NABTO_LOG_BUFFER(key, key_size);
        unabto_sha256_init(&sha_ctx);
        for (i = 0; i < keys_size; i++) {
            unabto_sha256_update(&sha_ctx, keys[i].data, keys[i].size);
        }
        unabto_sha256_final(&sha_ctx,key_temp);
        
        //sha256(key, key_size, key_temp);
        //NABTO_LOG_BUFFER(key_temp, SHA256_DIGEST_LENGTH);
        key_used = (const unsigned char*)key_temp;
    }
    fill = SHA256_BLOCK_LENGTH - num;

    memset(block_pad + num, 0x36, fill);
    for (i = 0; i < num; i++) {
        block_pad[i] = key_used[i] ^ 0x36;
    }
//    print_sha256_ctx(sha_ctx);
    unabto_sha256_init(&sha_ctx);
//    print_sha256_ctx(sha_ctx);
    unabto_sha256_update(&sha_ctx, block_pad, SHA256_BLOCK_LENGTH);
//    print_sha256_ctx(sha_ctx);
    for (i = 0; i < messages_size; i++) {
        unabto_sha256_update(&sha_ctx, messages[i].data, messages[i].size);
    }
//    print_sha256_ctx(sha_ctx);
    unabto_sha256_final(&sha_ctx, digest_temp);
//    print_sha256_ctx(sha_ctx);
    memset(block_pad + num, 0x5c, fill);
    for (i = 0; i < num; i++) {
        block_pad[i] = key_used[i] ^ 0x5c;
    }
   
    unabto_sha256_init(&sha_ctx);
//    print_sha256_ctx(sha_ctx);
    unabto_sha256_update(&sha_ctx, block_pad, SHA256_BLOCK_LENGTH);
//    print_sha256_ctx(sha_ctx);
    unabto_sha256_update(&sha_ctx, digest_temp, SHA256_DIGEST_LENGTH);
//    print_sha256_ctx(sha_ctx);
    unabto_sha256_final(&sha_ctx, digest_temp);
    
    UNABTO_ASSERT(mac_size <= 32 );
    memcpy(mac, (void*)digest_temp, mac_size);
}

#endif /* NABTO_ENABLE_UCRYPTO */
