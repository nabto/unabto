/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#include "unabto/unabto_env_base.h"

#if NABTO_ENABLE_UCRYPTO

#include "unabto/unabto_aes_cbc.h"
#include "modules/crypto/generic/unabto_aes.h"

#if UNABTO_PLATFORM_PIC18
#pragma udata big_mem
#endif

static AES_CTX ctx;

#if UNABTO_PLATFORM_PIC18
#pragma udata
#endif

bool unabto_aes128_cbc_encrypt(const uint8_t* key, uint8_t* input, uint16_t input_len) {
    if ((input_len < 16) || (input_len % 16 != 0)) {
        return false;
    }
    AES_set_key(&ctx, key, input, AES_MODE_128);
    input+=16; // the iv
    input_len -= 16;

    AES_cbc_encrypt(&ctx, input, input, (int)input_len);

    return true;
}

/**
 * we are running the algoritm backwards to eliminate the need to remember too much state.
 */
bool unabto_aes128_cbc_decrypt(const uint8_t* key, uint8_t* input, uint16_t input_len) {
    if (input_len < 16) return false; // the input contains at most the iv.
    if ((input_len % 16) != 0) return false; // the input_len should be a multiple of the block size.
    {
        AES_set_key(&ctx, key, input, AES_MODE_128);
        input += 16; // iv
        input_len -= 16;
        AES_convert_key(&ctx);
        AES_cbc_decrypt(&ctx, input, input, (int)input_len);
    }
    return true;
}

#endif /*NABTO_ENABLE_UCRYPTO*/
