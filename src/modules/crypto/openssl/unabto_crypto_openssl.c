/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#include <unabto/unabto_aes_cbc.h>
#include <unabto/unabto_hmac_sha256.h>
#include <unabto/unabto_util.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/core_names.h>

#include <string.h>

bool unabto_aes128_cbc_encrypt(const uint8_t* key, uint8_t* input, uint16_t input_len) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        NABTO_LOG_ERROR(("cannot allocate cipher ctx"));
    }
    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, input /* first 16 bytes of the input is the iv */) == 0) {
        NABTO_LOG_ERROR(("EVP_EncryptInit_ex should return 1"));
    }
    // unabto handles padding itself.
    EVP_CIPHER_CTX_set_padding(ctx, 0);

    int outLength;
    if (EVP_EncryptUpdate(ctx, input + 16, &outLength, input + 16, input_len - 16) != 1) {
        NABTO_LOG_ERROR(("EVP_EncryptUpdate should return 1"));
    }

    int tmpLength;
    if (EVP_EncryptFinal_ex(ctx, input + 16 + outLength, &tmpLength) != 1) {
        NABTO_LOG_ERROR(("EVP_EncryptFinal_ex should return 1"));
    }

    EVP_CIPHER_CTX_free(ctx);
    return true;
}

bool unabto_aes128_cbc_decrypt(const uint8_t* key, uint8_t* input, uint16_t input_len) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        NABTO_LOG_ERROR(("cannot allocate cipher ctx"));
    }
    if (EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, input /* iv is the first 16 bytes*/) != 1) {
        NABTO_LOG_ERROR(("EVP_DecryptInit_ex should return 1"));
    }
    // unabto handles padding itself.
    EVP_CIPHER_CTX_set_padding(ctx, 0);

    int outLength;
    if (EVP_DecryptUpdate(ctx, input + 16, &outLength, input + 16, input_len - 16) != 1) {
        NABTO_LOG_ERROR(("EVP_DecryptUpdate should return 1"));
    }

    int tmpLength;
    if (EVP_DecryptFinal_ex(ctx, input + 16 + outLength, &tmpLength) != 1) {
        NABTO_LOG_ERROR(("EVP_EncryptFinal_ex should return 1"));
    }

    EVP_CIPHER_CTX_free(ctx);
    return true;
}

void unabto_hmac_sha256_buffers(const unabto_buffer keys[], uint8_t keys_size,
                                const unabto_buffer messages[], uint8_t messages_size,
                                uint8_t* mac, uint16_t mac_size) {
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

    EVP_MAC* mac_alg = EVP_MAC_fetch(NULL, "HMAC", NULL);
    if (mac_alg == NULL) {
        NABTO_LOG_ERROR(("EVP_MAC_fetch failed"));
    }

    EVP_MAC_CTX* ctx = EVP_MAC_CTX_new(mac_alg);
    if (ctx == NULL) {
        NABTO_LOG_ERROR(("EVP_MAC_CTX_new failed"));
    }

    OSSL_PARAM params[] = {
        OSSL_PARAM_construct_utf8_string(OSSL_MAC_PARAM_DIGEST, "SHA256", 0),
        OSSL_PARAM_construct_end()};

    if (EVP_MAC_init(ctx, key, key_size, params) != 1) {
        NABTO_LOG_ERROR(("EVP_MAC_init failed"));
    }

    for (i = 0; i < messages_size; i++) {
        if (EVP_MAC_update(ctx, messages[i].data, messages[i].size) != 1) {
            NABTO_LOG_ERROR(("EVP_MAC_update failed"));
        }
    }

    size_t hash_size;
    if (EVP_MAC_final(ctx, hash, &hash_size, sizeof(hash)) != 1) {
        NABTO_LOG_ERROR(("EVP_MAC_final failed"));
    }

    memcpy(mac, hash, MIN(mac_size, hash_size));
    EVP_MAC_CTX_free(ctx);
    EVP_MAC_free(mac_alg);
}

bool unabto_constant_time_compare(const uint8_t* a, const uint8_t* b, size_t len) {
    return CRYPTO_memcmp(a, b, len) == 0;
}
