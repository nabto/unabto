/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_PRF_H_
#define _UNABTO_PRF_H_

#include <unabto/unabto_env_base.h>
#include <unabto/unabto_hmac_sha256.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * defines a prf based on the sha 256 hashing function as described in rfc4868 and rfc4306
 * The key is the secret, the seed comes from the nonces.
 */

bool prfplus_sha256(const unabto_buffer keys[],  uint8_t keys_size,
                    const unabto_buffer seeds[], uint8_t seeds_size,
                    uint8_t* data, uint16_t dataLength);



#define prf_sha256 unabto_hmac_sha256_buffers
/* void prf_sha256(const unabto_buffer keys[], uint16_t  keys_size, */
/*                 const unabto_buffer messages[], uint16_t  messages_size, */
/*                 uint8_t *mac, uint16_t  mac_size); */


#ifdef __cplusplus
} //extern "C"
#endif

#endif
