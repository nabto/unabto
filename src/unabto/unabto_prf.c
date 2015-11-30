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
 * this maps to the sha256 hmac function, but does some concatenation first
 * as described in rfc4306
 * @return number of bytes written to data
 */
static uint16_t prfhelper(const buffer_t keys[], uint8_t keys_size,
                 const buffer_t seeds[], uint8_t seeds_size,
                 const uint8_t* lastprf, uint16_t lastprflen,
                 uint8_t octet,
                 uint8_t* data, uint16_t datalen);

bool prfplus_sha256(const buffer_t keys[],  uint8_t keys_size,
                    const buffer_t seeds[], uint8_t seeds_size,
                    uint8_t* data, uint16_t dataLength) {
    uint8_t counter = 1;
    uint16_t  written;
//    uint16_t  dataLengthTmp = dataLength;
//    uint8_t *dataTmp = data;
    
    if (dataLength > 255*UNABTO_SHA256_DIGEST_LENGTH) // the ike v2 prf is only defined up to this.
    {
      return false;
    }

    written = prfhelper(keys, keys_size,
                        seeds, seeds_size,
                        NULL, 0,
                        counter++,
                        data, dataLength);
    data += written;
    dataLength -= written;

    while ( dataLength > 0 ) {
        written = prfhelper(keys, keys_size,
                            seeds, seeds_size,
                            data-UNABTO_SHA256_DIGEST_LENGTH, UNABTO_SHA256_DIGEST_LENGTH,
                            counter++,
                            data, dataLength);
        data += written;
        dataLength -= written;
    }



//    (void) dataTmp;
//    (void) dataLengthTmp;

    return true;    
}

uint16_t prfhelper(const buffer_t keys[], uint8_t keys_size,
                 const buffer_t seeds[], uint8_t seeds_size,
                 const uint8_t* prfoutput, uint16_t prfoutputlen,
                 uint8_t octet,
                 uint8_t* data, uint16_t datalen) {
    buffer_t seedMark[4];
    buffer_t octetBuffer;
    buffer_t* ptr = seedMark;
    uint8_t  i;
    uint16_t  realDataLen;

    UNABTO_ASSERT(seeds_size <= 2);

    buffer_init(ptr, (uint8_t*)prfoutput, prfoutputlen);
    ptr++;

    for (i = 0; i < seeds_size; i++) {
        *ptr = seeds[i];
        ptr++;
    }
    
    buffer_init(&octetBuffer, &octet, 1);
    
    *ptr = octetBuffer;
    ptr++;

    realDataLen = MIN(datalen, UNABTO_SHA256_DIGEST_LENGTH);

    unabto_hmac_sha256_buffers(keys, keys_size,
                               seedMark, (uint8_t)(ptr-seedMark),
                               data, realDataLen);
    
    return realDataLen;
}

#endif
