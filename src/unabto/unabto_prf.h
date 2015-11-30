/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_PRF_H_
#define _UNABTO_PRF_H_

#include <unabto/unabto_env_base.h>
#include <unabto/unabto_hmac_sha256.h>
#include <unabto/util/unabto_buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * defines a prf based on the sha 256 hashing function as described in rfc4868 and rfc4306
 * The key is the secret, the seed comes from the nonces.
 */

bool prfplus_sha256(const buffer_t keys[],  uint8_t keys_size,
                    const buffer_t seeds[], uint8_t seeds_size,
                    uint8_t* data, uint16_t dataLength);



#define prf_sha256 unabto_hmac_sha256_buffers
/* void prf_sha256(const buffer_t keys[], uint16_t  keys_size, */
/*                 const buffer_t messages[], uint16_t  messages_size, */
/*                 uint8_t *mac, uint16_t  mac_size); */


#ifdef __cplusplus
} //extern "C"
#endif

#endif
