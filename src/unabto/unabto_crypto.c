/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_ENCRYPTION

/**
 * @file
 * Nabto uServer crypto - Implementation.
 */
#include "unabto_env_base.h"

#if NABTO_ENABLE_CONNECTIONS

#include "unabto_crypto.h"
#include "unabto_context.h"
#include "unabto_main_contexts.h"
#include "unabto_util.h"
#include "unabto_memory.h"
#include "unabto_external_environment.h"


#if NABTO_ENABLE_UCRYPTO
#include "unabto_prf.h"
#include "unabto_hmac_sha256.h"
#include "unabto_aes_cbc.h"
#include "unabto_aes128_sha256.h"
#endif

#include <string.h>
#include <stdlib.h>


/******************************************************************************/

#if NABTO_ENABLE_REMOTE_ACCESS || NABTO_ENABLE_UCRYPTO
/**
 * Reset/initialise the Crypto context used in the attach sequence.
 * @param cryptoContext  the cryptographic context
 * @param ctx        the context
 */
static void nabto_crypto_reset_a(nabto_crypto_context* cryptoContext)
{
    switch(nmc.nabtoMainSetup.cryptoSuite) {
    case CRYPT_W_AES_CBC_HMAC_SHA256: {
#if NABTO_ENABLE_UCRYPTO
        unabto_buffer nonces[1];
        unabto_buffer seeds[1];
        
        unabto_buffer_init(&nonces[0], (uint8_t*)nmc.nabtoMainSetup.id, (uint16_t)strlen(nmc.nabtoMainSetup.id));
        unabto_buffer_init(&seeds[0], nmc.nabtoMainSetup.presharedKey, PRE_SHARED_KEY_SIZE);

        nabto_crypto_create_key_material(nonces, 1,
                                         seeds, 1,
                                         cryptoContext->key, KEY_MATERIAL_LENGTH);

        cryptoContext->code = CRYPT_W_AES_CBC_HMAC_SHA256;
        nabto_crypto_init_key(cryptoContext, false);

#else
        NABTO_LOG_FATAL(("aes chosen but no support is present"));
#endif
        break;
    } 
    case CRYPT_W_NULL_DATA: // nothing to do for this cipher here.
        break;
    }
}
#endif

/******************************************************************************/


#if NABTO_ENABLE_REMOTE_ACCESS
/**
 * Reset/initialise the Crypto context used in the GSP-device connection.
 * @param cryptoContext  the cryptographic context
 * @param ctx        the context
 * @param nonceGSP   nonce supplied from GSP (nonce from device is in *ctx)
 * @param seedUD     seed supplied by the device
 * @param seedGSP    seed supplied by the GSP
 */
static void nabto_crypto_reset_c(nabto_crypto_context* cryptoContext, const uint8_t* nonceGSP, const uint8_t* seedUD, const uint8_t* seedGSP)
{
#if NABTO_ENABLE_UCRYPTO
    switch(nmc.nabtoMainSetup.cryptoSuite) {
    case CRYPT_W_AES_CBC_HMAC_SHA256: {
        unabto_buffer nonces[2];
        unabto_buffer seeds[2];

        unabto_buffer_init(&nonces[0], (uint8_t*)nonceGSP, NONCE_SIZE);
        unabto_buffer_init(&nonces[1], (uint8_t*)nmc.context.nonceMicro, NONCE_SIZE);
        
        unabto_buffer_init(&seeds[0], (uint8_t*)seedGSP, SEED_SIZE);
        unabto_buffer_init(&seeds[1], (uint8_t*)seedUD, SEED_SIZE);

        nabto_crypto_create_key_material(
            nonces, 2,
            seeds, 2,
            cryptoContext->key, KEY_MATERIAL_LENGTH);
        cryptoContext->code = CRYPT_W_AES_CBC_HMAC_SHA256;
//        NABTO_LOG_BUFFER(("shared gsp key"),cryptoContext->key, KEY_MATERIAL_LENGTH);
        nabto_crypto_init_key(cryptoContext, false);

        break;
    }

    default:
        NABTO_LOG_TRACE(("crypto suite not supported %i", nmc.nabtoMainSetup.cryptoSuite));
    }
#endif
}
#endif


/******************************************************************************/

/**
 * Reset/initialise the Crypto context used in the connection/data between Client and device.
 * @param cryptoContext  the cryptographic context
 * @param code       the crypto suite code
 * @param key        the key material (simply copied from GSP)
 * @param keysize    the size of the key
 */
static void nabto_crypto_reset_d(nabto_crypto_context* cryptoContext, uint16_t code, const uint8_t* key, uint16_t keysize)
{
    switch(code) {
    case CRYPT_W_AES_CBC_HMAC_SHA256: {
#if NABTO_ENABLE_UCRYPTO
        if (code == CRYPT_W_AES_CBC_HMAC_SHA256) {
            if (keysize != KEY_MATERIAL_LENGTH) {
                NABTO_LOG_FATAL(("key material has the wrong size"));
            }
            memcpy(cryptoContext->key, key, KEY_MATERIAL_LENGTH);
            cryptoContext->code = code;
            nabto_crypto_init_key(cryptoContext, false);
        }
#else
        NABTO_LOG_FATAL(("no aes support"));
#endif
        break;
    }
    default:
        cryptoContext->code = CRYPT_W_NULL_DATA;
        break;
    }
}

/******************************************************************************/
/******************************************************************************/

#if NABTO_ENABLE_UCRYPTO

void nabto_crypto_create_key_material(const unabto_buffer nonces[], uint8_t nonces_size,
                                      const unabto_buffer seeds[],  uint8_t seeds_size,
                                      uint8_t* keyData, uint16_t keyDataLength)
{
    uint16_t skeyseed_len = 32;
    uint8_t skeyseed[32];
    unabto_buffer skeyseeds[1];

    prf_sha256(nonces, nonces_size, seeds, seeds_size, skeyseed, skeyseed_len);
    
    unabto_buffer_init(skeyseeds, skeyseed, skeyseed_len);

    prfplus_sha256(skeyseeds, 1, nonces, nonces_size, keyData, keyDataLength);
}

bool nabto_crypto_init_key(nabto_crypto_context* cryptoContext, bool initiator) {
    /**
     * we need 2x256bit for hmac integrity checks and 2x128bit for shared aes keys.
     * 96 bytes of keying material.
     *
     * The key layout is as follows, first the shared symmetric keys and then the shared hmac keys.
     */
    if (initiator) {
        cryptoContext->encryptkey = cryptoContext->key;
        cryptoContext->decryptkey = cryptoContext->key+CRYPT_KEY_LENGTH;
        cryptoContext->ourhmackey = cryptoContext->key+2*CRYPT_KEY_LENGTH;
        cryptoContext->theirhmackey = cryptoContext->key+2*CRYPT_KEY_LENGTH+HMAC_KEY_LENGTH;
    } else {
        cryptoContext->decryptkey = cryptoContext->key;
        cryptoContext->encryptkey = cryptoContext->key+CRYPT_KEY_LENGTH;
        cryptoContext->theirhmackey = cryptoContext->key+2*CRYPT_KEY_LENGTH;
        cryptoContext->ourhmackey = cryptoContext->key+2*CRYPT_KEY_LENGTH+HMAC_KEY_LENGTH;
    }
    return true;
/* #else */
/*     NABTO_NOT_USED(cryptoContext); */
/*     NABTO_NOT_USED(initiator); */
/*     return false; */
/* #endif */
}

#endif // NABTO_ENABLE_UCRYPTO

void nabto_crypto_reset(nabto_crypto_context* cryptoContext)
{
    cryptoContext->code = CRYPT_W_NULL_DATA;
}

void nabto_crypto_init(nabto_crypto_context* cryptoContext, crypto_application cryptoApplication)
{
    crypto_suite suite = CRYPT_W_NULL_DATA;
    suite = nmc.nabtoMainSetup.cryptoSuite;

    switch(suite) {
    case CRYPT_W_AES_CBC_HMAC_SHA256: {
#if NABTO_ENABLE_UCRYPTO
    cryptoContext->code   = 0;
    switch (cryptoApplication) {
        case CRYPTO_A:
            nabto_crypto_reset_a(cryptoContext);
            break;
        case CRYPTO_C:
        case CRYPTO_D:
            // Nothing to do for them here.
            break;
    }
#else 
        NABTO_LOG_FATAL(("no aes support"));
#endif
        break;
    }
    default:
    cryptoContext->code = CRYPT_W_NULL_DATA;
        break;
    }
}

/******************************************************************************/

void nabto_crypto_release(nabto_crypto_context* cryptoContext)
{
    switch(cryptoContext->code) {
    case CRYPT_W_AES_CBC_HMAC_SHA256:
    memset(cryptoContext, 0, sizeof(nabto_crypto_context));
        break;
    default:
    cryptoContext->code = CRYPT_W_NULL_DATA;
    }
}

/******************************************************************************/

void nabto_crypto_reinit_a(void)
{
#if NABTO_ENABLE_REMOTE_ACCESS
    nabto_crypto_release(nmc.context.cryptoAttach);
    nabto_crypto_reset_a(nmc.context.cryptoAttach);
#endif
}

/******************************************************************************/

void unabto_crypto_reinit_c(const uint8_t* nonceGSP, const uint8_t* seedUD, const uint8_t* seedGSP)
{
#if NABTO_ENABLE_REMOTE_ACCESS
    nabto_crypto_release(nmc.context.cryptoConnect);
    nabto_crypto_reset_c(nmc.context.cryptoConnect, nonceGSP, seedUD, seedGSP);
#else
    NABTO_NOT_USED(nonceGSP);
    NABTO_NOT_USED(seedUD);
    NABTO_NOT_USED(seedGSP);
#endif
}

/******************************************************************************/

void unabto_crypto_reinit_d(nabto_crypto_context* cryptoContext, crypto_suite cryptoSuite, const uint8_t* key, uint16_t keysize)
{
    nabto_crypto_release(cryptoContext);
    nabto_crypto_reset_d(cryptoContext, cryptoSuite, key, keysize);
}

/******************************************************************************/

uint16_t unabto_crypto_max_data(nabto_crypto_context* cryptoContext, uint16_t available)
{
    uint16_t l_iv_integ;
    uint16_t l_block;
    switch(cryptoContext->code) {
        case CRYPT_W_AES_CBC_HMAC_SHA256:
            l_iv_integ = 16 + 16; // reduce for IV and integrity
            l_block    = 16;
            break;
        default:
            l_iv_integ = 0 + 2; // reduce for IV and integrity
            l_block    = 2;
            break;
    }
    if (available < l_block + l_iv_integ) return 0;
    available -= l_iv_integ;
    return (available / l_block) * l_block - 1;
}

/******************************************************************************/

uint16_t unabto_crypto_required_length(nabto_crypto_context* cryptoContext, uint16_t size)
{
    switch (cryptoContext->code) {
        case CRYPT_W_AES_CBC_HMAC_SHA256: return 16 + (size/16 + 1)*16 + 16; // IV + padded_data + Integrity
        default:                          return 0  + (size/2  + 1)*2  + 2;  // do
    }
}

/******************************************************************************/

bool unabto_verify_integrity(nabto_crypto_context* cryptoContext, uint16_t code, const uint8_t* buf, uint16_t size, uint16_t* verifSize)
{
    if (code != cryptoContext->code) {
        NABTO_LOG_TRACE(("Encryption code inconsistency(1): rcvd: 0x%x required 0x%x", code, cryptoContext->code));
        return false;
    }
#if NABTO_ENABLE_UCRYPTO
    if (code == CRYPT_W_AES_CBC_HMAC_SHA256) {
        bool ret;
        if (size < TRUNCATED_HMAC_SHA256_LENGTH) {
            NABTO_LOG_TRACE(("Size less than truncated MAC length"));
            return false;
        }
        *verifSize = TRUNCATED_HMAC_SHA256_LENGTH;
        //NABTO_LOG_TRACE(("calculating truncated hmac"));
        //NABTO_LOG_BUFFER(cryptoContext->theirhmackey, HMAC_SHA256_KEYLENGTH);
        //NABTO_LOG_BUFFER(buf, size-TRUNCATED_HMAC_SHA256_LENGTH);
        //NABTO_LOG_BUFFER(buf+size-TRUNCATED_HMAC_SHA256_LENGTH, TRUNCATED_HMAC_SHA256_LENGTH);
        ret = unabto_truncated_hmac_sha256_verify_integrity(
            cryptoContext->theirhmackey, HMAC_SHA256_KEYLENGTH,
            buf, size - TRUNCATED_HMAC_SHA256_LENGTH,
            buf+size-TRUNCATED_HMAC_SHA256_LENGTH);
        //NABTO_LOG_TRACE(("ret %u", ret));
        return ret;
    }
#endif

    if (code != CRYPT_W_NULL_DATA) {
        NABTO_LOG_TRACE(("Verification code %" PRIu16 " not supported", code));
        return false;
    } else {
        uint16_t sum = 0;
        const uint8_t* begin = buf;
        const uint8_t* endx  = buf + size - 2;
        uint16_t sumt;
        while (begin < endx) sum += *begin++;
        READ_U16(sumt, endx);
        if (sum != sumt) {
            NABTO_LOG_TRACE(("Verification failure: %" PRIu16 "/%" PRIu16, sumt, sum));
            return false;
        } else {
            *verifSize = 2;
        }
    }
    return true;
}

/******************************************************************************/

bool unabto_decrypt(nabto_crypto_context* cryptoContext, uint8_t* ptr, uint16_t size, uint16_t *decrypted_size)
{
    bool res = false;
    switch(cryptoContext->code) {
    case CRYPT_W_AES_CBC_HMAC_SHA256: {
#if NABTO_ENABLE_UCRYPTO
    /**
     * we have a hmac verified packet of some multiple of 16 in length, we need to make an
     * inbuffer decrypt and return the length without the padding.
     */
        if (!unabto_aes128_cbc_decrypt(cryptoContext->decryptkey,
                             ptr, size)) {
            NABTO_LOG_TRACE(("Decrypting of packet failed size: %u", size));
        }
        else
        {
            uint8_t padding_length;
            READ_U8(padding_length, (ptr+size-1));
            if (padding_length > 16) {
                NABTO_LOG_TRACE(("Padding longer than 16 bytes"));
                break;
            }
            
            if (size-16 < padding_length) {
                NABTO_LOG_TRACE(("Packet to short for padding"));
                break;
            }
            *decrypted_size = size-16-padding_length;
            
            memmove(ptr,(const uint8_t*)(ptr+16), *decrypted_size);          
            res = true;
            break;
        }
#else
        NABTO_LOG_FATAL(("aes not supported"));
#endif
        break;
    }
    default:
        {
            uint8_t pad;
            READ_U8(pad, ptr + size - 1);
            if (pad > size) {
                NABTO_LOG_TRACE(("Padding error: %" PRIu8 " of %" PRIu16, pad, size));
            }
            else
            {
                *decrypted_size = size - pad;
                res = true;
            }
        }
    }
    return res;
}

/******************************************************************************/

bool unabto_encrypt(nabto_crypto_context* cryptoContext, const uint8_t* src, uint16_t size, uint8_t* dst, uint8_t* dstEnd, uint8_t **encryptedEnd)
{
    bool res = false;
    switch(cryptoContext->code) {
    case CRYPT_W_AES_CBC_HMAC_SHA256: {
 #if NABTO_ENABLE_UCRYPTO
        /**
         * We are using an encrypt then mac scheme
         * The encrypted packet with integrity is iv+E_K(data+padding)+HMAC(iv+E_K(data+padding))
         * the HMAC is 16 bytes, the padding is 1-16 bytes the length is described in the last byte.
         */
        {
         uint8_t* ptr = dst;
         uint16_t paddingSize;
         size_t encryptedSize;
         encryptedSize = (16+(size/16 + 1)*16+16); // IV + padded_data + Integrity
         if ((dstEnd - dst) < (ptrdiff_t)encryptedSize) { // dst size too small to hold packet
             break;
         }
         *encryptedEnd = dst + encryptedSize;
         // Write length to payload header   
         WRITE_U16(dst - SIZE_CODE - 2, (uint16_t)(SIZE_PAYLOAD_HEADER + SIZE_CODE + encryptedSize)); // 2 is packet length field
            
         // First move the data because src and dst may overlap
         ptr += 16; // iv length
         memmove(ptr, src, size); ptr += size;        
            
         // Write crypto code to payload
         WRITE_U16(dst - SIZE_CODE, cryptoContext->code);
         
         // put the iv into the start of the buffer
         nabto_random(dst, 16);
            
         paddingSize = (size/16+1)*16 - size;
         // insert the padding length on all the padding bytes
         memset(ptr, (int)paddingSize, paddingSize); ptr += paddingSize;
            
         UNABTO_ASSERT(ptr - dst <= 0xFFFF);
         res = unabto_aes128_cbc_encrypt(cryptoContext->encryptkey, dst, (uint16_t)(ptr-dst));
        }
#else
        NABTO_LOG_FATAL(("aes cryptosuite not implemented"));
#endif
        break;
    }
    default: {
        uint8_t pad;
        size_t encryptedSize = ((size/2 + 1) * 2) + 2; /* including integrity */
        *encryptedEnd = dst + encryptedSize;
        if ((ptrdiff_t)(encryptedSize) > (dstEnd - dst)) {
            NABTO_LOG_TRACE(("Encryption Overflow %" PRIsize " > %" PRIsize,  encryptedSize, (size_t)(dstEnd - dst)));
        }
        else
        {
            // Write payload length
            WRITE_U16(dst - 4, (uint16_t)(SIZE_PAYLOAD_HEADER + SIZE_CODE + encryptedSize)); // add 2 for integrity
            if (size && src != dst) {
                memmove(dst, src, size);
            }
            // Write crypto code to payload
            WRITE_U16(dst - 2, CRYPT_W_NULL_DATA);
            pad = (uint8_t)((encryptedSize-2) - size);
            memset(dst + size, pad, pad);
            res = true;
        }
        break;
      }
    }
    return res;
}

/******************************************************************************/
/******************************************************************************/

bool unabto_insert_integrity(nabto_crypto_context* cryptoContext, uint8_t* start, uint16_t plen)
{
    bool res=false;
    switch(cryptoContext->code) {
    case CRYPT_W_AES_CBC_HMAC_SHA256: {
        NABTO_LOG_TRACE(("CRYPT_W_AES_CBC_HMAC_SHA256"));
#if NABTO_ENABLE_UCRYPTO
        // insert 16 byte hmac at the end of the buffer
        if (plen < 16) {
            NABTO_LOG_ERROR(("no room for hmac"));
        } else {
            unabto_buffer keys[1];
            unabto_buffer messages[1];
            
            unabto_buffer_init(keys, cryptoContext->ourhmackey, HMAC_KEY_LENGTH);
            unabto_buffer_init(messages, start, plen-16);

            unabto_hmac_sha256_buffers(keys, 1,
                                       messages, 1,
                                       start+plen-16, 16);
            res = true;
            break;
        }
#else 
        NABTO_LOG_FATAL(("aes not supported"));
#endif
        break;
      }
   default: {
        uint16_t sum = 0;
        uint8_t* begin = start;
        uint8_t* end   = start + plen - 2;
        NABTO_LOG_TRACE(("Crypto default!"));
        while (begin < end) sum += *begin++;
        WRITE_U16(end, sum);
        res = true;
      }
    }

    return res;
}

bool unabto_crypto_verify_and_decrypt(nabto_packet_header* hdr,
                                      nabto_crypto_context* cryptoContext,
                                      struct unabto_payload_crypto* crypto,
                                      uint8_t** decryptedDataBegin,
                                      uint16_t* decryptedDataLength)
{
    uint16_t verifSize;
    uint16_t dlen;
    
    if (!unabto_verify_integrity(cryptoContext, crypto->code, nabtoCommunicationBuffer, hdr->len, &verifSize)) {
        NABTO_LOG_TRACE(("Could not verify integrity in packet type %i", hdr->type));
        return false;
    }

    
    if (!unabto_decrypt(cryptoContext, (uint8_t*)crypto->dataBegin, ((crypto->dataEnd - crypto->dataBegin) - verifSize), &dlen)) {
        NABTO_LOG_TRACE(("Decryption fail in packet type %i", hdr->type));
        return false;
    }
    
    *decryptedDataLength = dlen;
    *decryptedDataBegin = (uint8_t*)crypto->dataBegin;
    return true;
}




/******************************************************************************/

#endif
