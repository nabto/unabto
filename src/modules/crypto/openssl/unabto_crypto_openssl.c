/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#include <unabto/unabto_aes_cbc.h>
#include <unabto/unabto_hmac_sha256.h>
#include <unabto/util/unabto_buffer.h>
#include <unabto/unabto_util.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>

#include <string.h>

bool aes128_cbc_encrypt(const uint8_t* key, uint8_t* input, uint16_t input_len)
{
    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX_init(&ctx);
    EVP_EncryptInit_ex(&ctx, EVP_aes_128_cbc(), NULL, key, input /* first 16 bytes of the input is the iv */);

    int outLength;
    EVP_EncryptUpdate(&ctx, input+16, &outLength, input+16, input_len-16);
    EVP_CIPHER_CTX_cleanup(&ctx);
    return true;
}

bool aes128_cbc_decrypt(const uint8_t* key, uint8_t* input, uint16_t input_len)
{
    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX_init(&ctx);
    EVP_DecryptInit_ex(&ctx, EVP_aes_128_cbc(), NULL, key, input /* iv is the first 16 bytes*/);
    
    int outLength;
    EVP_DecryptUpdate(&ctx, input+16, &outLength, input+16, input_len-16);
    EVP_CIPHER_CTX_cleanup(&ctx);
    return true;
}

void unabto_hmac_sha256_buffers(const buffer_t keys[], uint8_t keys_size,
                                const buffer_t messages[], uint8_t messages_size,
                                uint8_t *mac, uint16_t mac_size) {
    uint16_t key_size = 0;
    uint8_t i;
    uint8_t* key;

    if (keys_size == 1) {
        key = keys[0].data;
        key_size = keys[0].size;
    } else {
        uint8_t key_buffer[UNABTO_SHA256_BLOCK_LENGTH];
        
        for (i = 0; i < keys_size; i++) {
            key_size += keys[i].size;
        }
        
        UNABTO_ASSERT(key_size <= UNABTO_SHA256_BLOCK_LENGTH);
        
        uint8_t* key_ptr = key_buffer;
        for (i = 0; i < keys_size; i++) {
            memcpy(key_ptr, keys[i].data, keys[i].size);
            key_ptr += keys[i].size;
        }
        key = key_buffer;
    }

    uint8_t hash[EVP_MAX_MD_SIZE];
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    
    HMAC_Init(&ctx, key, key_size, EVP_sha256());

    for (i = 0; i < messages_size; i++) {
        HMAC_Update(&ctx, messages[i].data, messages[i].size);
    }

    unsigned int hash_size;
    HMAC_Final(&ctx, hash, &hash_size);

    memcpy(mac, hash, MIN(mac_size, hash_size));
    HMAC_CTX_cleanup(&ctx);
}
