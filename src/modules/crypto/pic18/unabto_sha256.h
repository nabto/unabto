#ifndef _UNABTO_SHA256_H_
#define _UNABTO_SHA256_H_

#define SHA256_BLOCK_LENGTH 64
#define SHA256_DIGEST_LENGTH 32

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;

/**
 * Initialize the sha256 context
 */
void unabto_sha256_init(void);

/**
 * Update the sha256 context with the given data
 */
void unabto_sha256_update(const uint8_t* data, uint16_t length);

void unabto_sha256_final(uint8_t* digest);

#endif
