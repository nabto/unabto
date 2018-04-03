/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
/**
 * @file
 * Nabto uServer crypto - Interface.
 *
 */

#ifndef _UNABTO_CRYPTO_H_
#define _UNABTO_CRYPTO_H_

#include <unabto/unabto_env_base.h>
#include <unabto/unabto_packet_util.h>
#include <unabto/unabto_buffers.h>

#if NABTO_ENABLE_CONNECTIONS

#ifdef __cplusplus
extern "C" {
#endif

#if NABTO_ENABLE_UCRYPTO
enum {
    KEY_MATERIAL_LENGTH = 96,
    HMAC_KEY_LENGTH = 32,
    CRYPT_KEY_LENGTH = 16,
    TRUNCATED_HMAC_LENGTH = 16
};
#endif // NABTO_ENABLE_UCRYPTO

/** the type of a crypto context (the crypto application) */
typedef enum {
    CRYPTO_A, /**< for U_ATTACH      */
    CRYPTO_C, /**< for U_CONNECT etc */
    CRYPTO_D  /**< for DATA          */
} crypto_application;


/// The micro crypto context
typedef struct {
    uint16_t code;          /**< algorithm code        */

#if NABTO_ENABLE_UCRYPTO
    uint8_t  key[96];       /**< the concatenated keys */
    uint8_t* ourhmackey;    /**< our hmac key          */
    uint8_t* theirhmackey;  /**< their hmac key        */
    uint8_t* encryptkey;    /**< key for encryption    */
    uint8_t* decryptkey;    /**< key for decryption    */
#endif

} nabto_crypto_context;

#if NABTO_ENABLE_UCRYPTO

/**
 * construct the key material from nonces and secrets
 * @param nonces        the concatenated nonces
 * @param noncesLength  the length of the concatenated nonces
 * @param seeds         the concatenated seeds
 * @param seedsLength   the length of the concatenated seeds
 * @param data          the data
 * @param dataLength    the length of the data
 */
void nabto_crypto_create_key_material(const unabto_buffer nonces[], uint8_t nonces_size,
                                      const unabto_buffer seeds[],  uint8_t seeds_size,
                                      uint8_t* data, uint16_t dataLength);

/**
 * initialise a cryptocontext given the key material
 * @param cryptoContext  the cryptocontext
 * @param initiator  true iff called by the initiator
 * @return           true if successfully initialised
 */
bool nabto_crypto_init_key(nabto_crypto_context* cryptoContext, bool initiator);

#endif // NABTO_ENABLE_UCRYPTO

/**
 * Reset a crypto context
 */
void nabto_crypto_reset(nabto_crypto_context* cryptoContext);

/**
 * Initialise a crypto context at start of the program.
 * @param cryptoContext  the cryptocontext
 * @param cryptoApplication        the type of the context
 */
void nabto_crypto_init(nabto_crypto_context* cryptoContext, crypto_application cryptoApplication);


/**
 * Release a cryptocontext.
 * @param cryptoContext  the cryptocontext
 */
void nabto_crypto_release(nabto_crypto_context* cryptoContext);


/**
 * Re-initialise the type A crypto context before reattaching to the GSP.
 */
void nabto_crypto_reinit_a(void);


/**
 * Re-initialise the type C crypto context during attach to the GSP.
 * @param nonceGSP  the nonce supplied from the GSP
 * @param seedUD    the nonce sent to the GSP
 * @param seedGSP   the seed supplied from the GSP (secret)
 */
void unabto_crypto_reinit_c(const uint8_t* nonceGSP, const uint8_t* seedUD, const uint8_t* seedGSP);


/**
 * Re-initialise a type D (data) crypto context during connection establishment.
 * @param cryptoContext  the cryptocontext
 * @param key        the key supplied from the GSP (secret)
 * @param keySize    the size of the key
 */
void unabto_crypto_reinit_d(nabto_crypto_context* cryptoContext, crypto_suite cryptoSuite, const uint8_t* key, uint16_t keysize);


/**
 * Query the number of bytes available for data to be encrypted and integrity-protected.
 * @param cryptoContext  the cryptocontext
 * @param available  the size available for IV, padded data and integrity-sum.
 * @return           the max number of clear text data to be sent in the available space.
 */
uint16_t unabto_crypto_max_data(nabto_crypto_context* cryptoContext, uint16_t available);


/**
 * Calculate the required length for encryption and integrity data.
 * @param cryptoContext  the cryptocontext
 * @param size       the lengtn of the clear text data
 * @return           the number of bytes required
 */
uint16_t unabto_crypto_required_length(nabto_crypto_context* cryptoContext, uint16_t size);


/**
 * Verify the integrity of a packet and the correctness of the algorithm code.
 * @param cryptoContext  the cryptocontext
 * @param code       the algoritm code
 * @param buf        the start of the packet
 * @param size       the size of the packet
 * @param verifSize  to return the length in bytes of the MAC.
 * @return           true iff the packet is verified
 */
bool unabto_verify_integrity(nabto_crypto_context* cryptoContext, uint16_t code, const uint8_t* buf, uint16_t size, uint16_t* verifSize);


/**
 * Decrypt a CRYPTO payload.
 * @param cryptoContext      the cryptocontext
 * @param ptr            the start of the encrypted buffer (and the decrypted as well)
 * @param size           the size of the encrypted buffer
 * @param decryptedSize the length of the decrypted data
 * @return               true if successfully
 *
 * The decrypted data is overwriting the encrypted data.
 */
bool unabto_decrypt(nabto_crypto_context* cryptoContext, uint8_t* ptr, uint16_t size, uint16_t *decryptedSize);

/**
 * Verify and decrypt CRYPTO payload
 * @param cryptoContext
 * @param cryptoPayloadStart
 * @param cryptoPayloadEnd
 * @param decryptedDataBegin
 * @param decryptedDataLength
 * @return true iff the verification and decryption was successful
 */
bool unabto_crypto_verify_and_decrypt(nabto_packet_header* hdr,
                                      nabto_crypto_context* cryptoContext,
                                      struct unabto_payload_crypto* crypto,
                                      uint8_t** decryptedDataBegin,
                                      uint16_t* decryptedDataLength);

/**
 * Encrypt a CRYPTO payload and write the payload length and algoritm code into the payload.
 * Src and dst may overlap so move data before e.g. an iv is written to dst.
 * @param cryptoContext   the cryptocontext
 * @param src         the clear text data
 * @param size        the size of the clear text data
 * @param dst         the location to put the encrypted data this is the pointer to the data in the crypto payload ie. cryptoPayloadStart + 4 (payload header) + 2 (crypto code).
 * @param dstsize     the size of the destination buffer (must be large enough to the later appended verification)
 * @param encryptedEnd the end of the encrypted data including the integrity value.
 * @return            true if successfully
 */
bool unabto_encrypt(nabto_crypto_context* cryptoContext, const uint8_t* src, uint16_t size, uint8_t* dst, uint8_t* dstEnd, uint8_t **encryptedEnd);


/**
 * Insert integrity value into a packet.
 * @param cryptoContext  the cryptocontext
 * @param start      start of the packet
 * @param plen       length of the packet including the (not yet inserted) integrity value
 * @return           true if successfully
 */
bool unabto_insert_integrity(nabto_crypto_context* cryptoContext, uint8_t* start, uint16_t plen);


#ifdef __cplusplus
} //extern "C"
#endif

#endif

#endif
