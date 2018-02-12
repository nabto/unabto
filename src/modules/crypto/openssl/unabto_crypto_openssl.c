/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#include <unabto/unabto_aes_cbc.h>
#include <unabto/unabto_hmac_sha256.h>
#include <unabto/unabto_util.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>

#include <string.h>

bool unabto_aes128_cbc_encrypt(const uint8_t* key, uint8_t* input, uint16_t input_len)
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        NABTO_LOG_ERROR(("cannot allocate cipher ctx"));
    }
    EVP_CIPHER_CTX_init(ctx);
    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, input /* first 16 bytes of the input is the iv */) == 0) {
        NABTO_LOG_ERROR(("EVP_EncryptInit_ex should return 1"));
    }
    // unabto handles padding itself.
    EVP_CIPHER_CTX_set_padding(ctx, 0);

    int outLength;
    if(EVP_EncryptUpdate(ctx, input+16, &outLength, input+16, input_len-16) != 1) {
        NABTO_LOG_ERROR(("EVP_EncryptUpdate should return 1"));
    }

    int tmpLength;
    if(EVP_EncryptFinal_ex(ctx, input + 16 + outLength, &tmpLength) != 1) {
        NABTO_LOG_ERROR(("EVP_EncryptFinal_ex should return 1"));
    }

    if(EVP_CIPHER_CTX_cleanup(ctx) != 1) {
        NABTO_LOG_ERROR(("EVP_CIPHER_CTX_cleanup should return 1"));
    }
    EVP_CIPHER_CTX_free(ctx);
    return true;
}

bool unabto_aes128_cbc_decrypt(const uint8_t* key, uint8_t* input, uint16_t input_len)
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        NABTO_LOG_ERROR(("cannot allocate cipher ctx"));
    }
    EVP_CIPHER_CTX_init(ctx);
    if (EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, input /* iv is the first 16 bytes*/) != 1) {
        NABTO_LOG_ERROR(("EVP_DecryptInit_ex should return 1"));
    }
    // unabto handles padding itself.
    EVP_CIPHER_CTX_set_padding(ctx, 0);

    int outLength;
    if (EVP_DecryptUpdate(ctx, input+16, &outLength, input+16, input_len-16) != 1) {
        NABTO_LOG_ERROR(("EVP_DecryptUpdate should return 1"));
    }

    int tmpLength;
    if(EVP_DecryptFinal_ex(ctx, input + 16 + outLength, &tmpLength) != 1) {
        NABTO_LOG_ERROR(("EVP_EncryptFinal_ex should return 1"));
    }

    if (EVP_CIPHER_CTX_cleanup(ctx) != 1) {
        NABTO_LOG_ERROR(("EVP_CIPHER_CTX_cleanup should return 1"));
    }
    EVP_CIPHER_CTX_free(ctx);
    return true;
}

void unabto_hmac_sha256_buffers(const unabto_buffer keys[], uint8_t keys_size,
                                const unabto_buffer messages[], uint8_t messages_size,
                                uint8_t *mac, uint16_t mac_size) {
    uint16_t key_size = 0;
    uint8_t i;
    uint8_t* key;
    uint8_t key_buffer[UNABTO_SHA256_BLOCK_LENGTH];
    if (keys_size == 1) {
        key = keys[0].data;
        key_size = keys[0].size;
    } else {
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

    HMAC_CTX* ctx;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    HMAC_CTX ctxData;
    ctx = &ctxData;
    HMAC_CTX_init(ctx);
#else
    ctx = HMAC_CTX_new();
#endif
    if (ctx == NULL) {
        NABTO_LOG_ERROR(("cannot allocate hmac ctx"));
    }
    
    if (HMAC_Init_ex(ctx, key, key_size, EVP_sha256(), NULL) != 1) {
        NABTO_LOG_ERROR(("HMAC_Init_ex should return 1"));
    }

    for (i = 0; i < messages_size; i++) {
        if (HMAC_Update(ctx, messages[i].data, messages[i].size) != 1) {
            NABTO_LOG_ERROR(("HMAC_Update should return 1"));
        }
    }

    unsigned int hash_size;
    if (HMAC_Final(ctx, hash, &hash_size) != 1) {
        NABTO_LOG_ERROR(("HMAC_Final should return 1"));
    }

    memcpy(mac, hash, MIN(mac_size, hash_size));
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    HMAC_CTX_cleanup(ctx);
#else
    HMAC_CTX_free(ctx);
#endif
}
