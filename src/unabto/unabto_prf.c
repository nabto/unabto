/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_ENCRYPTION

#include "unabto_env_base.h"

#if NABTO_ENABLE_UCRYPTO

#include "unabto_util.h"
#include "unabto_prf.h"
#include "unabto_hmac_sha256.h"
#include <string.h>


/**
 * This module calculates the prf+ found in IKEv2. See rfc4306 for reference.
 * prf+ (K,S) = T1 | T2 | T3 | T4 | ...
 *  where:
 *   T1 = prf (K, S | 0x01)
 *   T2 = prf (K, T1 | S | 0x02)
 *   T3 = prf (K, T2 | S | 0x03)
 *   T4 = prf (K, T3 | S | 0x04)
 */


/**
 * This function calculates TN.
 *
 * @param oldTn     Empty in the first round.
 * @param oldTnLen  0 in the first round.
 * @param datalen   max data to write to the data pointer.
 * @return  number of bytes written to data
 */
static uint16_t tn(const unabto_buffer keys[], uint8_t keys_size,
                   const unabto_buffer seeds[], uint8_t seeds_size,
                   const uint8_t* oldTn, uint16_t oldTnLen,
                   uint8_t octet,
                   uint8_t* data, uint16_t datalen);


bool prfplus_sha256(const unabto_buffer keys[],  uint8_t keys_size,
                    const unabto_buffer seeds[], uint8_t seeds_size,
                    uint8_t* data, uint16_t dataLength) {
    uint8_t counter = 1;
    uint16_t  written;

    // the ike v2 prf is only defined up to this.
    UNABTO_ASSERT(dataLength <= 255*UNABTO_SHA256_DIGEST_LENGTH);

    written = tn(keys, keys_size,
                 seeds, seeds_size,
                 NULL, 0,
                 counter++,
                 data, dataLength);
    data += written;
    dataLength -= written;

    while ( dataLength > 0 ) {
        written = tn(keys, keys_size,
                     seeds, seeds_size,
                     data-UNABTO_SHA256_DIGEST_LENGTH, UNABTO_SHA256_DIGEST_LENGTH,
                     counter++,
                     data, dataLength);
        data += written;
        dataLength -= written;
    }
    return true;    
}

uint16_t tn(const unabto_buffer keys[], uint8_t keys_size,
            const unabto_buffer seeds[], uint8_t seeds_size,
            const uint8_t* prfoutput, uint16_t prfoutputlen,
            uint8_t octet,
            uint8_t* data, uint16_t datalen)
{
    unabto_buffer seedMark[4];
    unabto_buffer octetBuffer;
    unabto_buffer* ptr = seedMark;
    uint8_t  i;
    uint16_t  realDataLen;

    UNABTO_ASSERT(seeds_size <= 2);

    unabto_buffer_init(ptr, (uint8_t*)prfoutput, prfoutputlen);
    ptr++;

    for (i = 0; i < seeds_size; i++) {
        *ptr = seeds[i];
        ptr++;
    }
    
    unabto_buffer_init(&octetBuffer, &octet, 1);
    
    *ptr = octetBuffer;
    ptr++;

    realDataLen = MIN(datalen, UNABTO_SHA256_DIGEST_LENGTH);

    unabto_hmac_sha256_buffers(keys, keys_size,
                               seedMark, (uint8_t)(ptr-seedMark),
                               data, realDataLen);

    return realDataLen;
}

#endif
