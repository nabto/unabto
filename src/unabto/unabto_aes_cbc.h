/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _AES_CBC_H_
#define _AES_CBC_H_

#include <unabto/unabto_env_base.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * the iv is the first 16 bytes of the input!
 * the resulting buffer can be send directly as a structure iv + E_K(input+16)
 */
bool unabto_aes128_cbc_encrypt(const uint8_t* key, uint8_t* input, uint16_t inputLength);

/**
 * the iv is the first 16 bytes of the input taken directly from a communication buffer containing
 * an iv and the data
 */
bool unabto_aes128_cbc_decrypt(const uint8_t* key, uint8_t* input, uint16_t inputLength);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
