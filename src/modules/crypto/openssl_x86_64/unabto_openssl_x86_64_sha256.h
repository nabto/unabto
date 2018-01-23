#ifndef _UNABTO_OPENSSL_SHA256_MIPS_H_
#define _UNABTO_OPENSSL_SHA256_MIPS_H_

#include <unabto_platform_types.h>

# define SHA_LBLOCK      16
# define SHA_CBLOCK      (SHA_LBLOCK*4)

# define SHA256_CBLOCK   (SHA_LBLOCK*4)/* SHA-256 treats input data as a
                                        * contiguous array of 32 bit wide
                                        * big-endian values. */
# define SHA256_DIGEST_LENGTH    32
# define SHA256_BLOCK_LENGTH     64

#define SHA_LONG uint32_t

typedef struct SHA256state_st {
    SHA_LONG h[8];
    SHA_LONG Nl, Nh;
    SHA_LONG data[SHA_LBLOCK];
    unsigned int num, md_len;
} SHA256_CTX;

int SHA256_Init_unabto(SHA256_CTX *c);
int SHA256_Update_unabto(SHA256_CTX *c, const void *data, size_t len);
int SHA256_Final_unabto(unsigned char *md, SHA256_CTX *c);
void SHA256_Transform_unabto(SHA256_CTX *c, const unsigned char *data);


#endif
