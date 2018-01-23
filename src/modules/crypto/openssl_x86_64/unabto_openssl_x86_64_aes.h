#ifndef _UNABTO_OPENSSL_X86_64_AES_H_
#define _UNABTO_OPENSSL_X86_64_AES_H_

#include <unabto_platform_types.h>

# define AES_MAXNR 14
# define AES_BLOCK_SIZE 16

/* This should be a hidden type, but EVP requires that the size be known */
struct aes_key_st {
    uint32_t rd_key[4 * (AES_MAXNR + 1)];
    int rounds;
};
typedef struct aes_key_st AES_KEY;

int aesni_set_encrypt_key(const unsigned char *userKey, const int bits,
                        AES_KEY *key);
int aesni_set_decrypt_key(const unsigned char *userKey, const int bits,
                        AES_KEY *key);

void aesni_encrypt(const unsigned char *in, unsigned char *out,
                 const AES_KEY *key);
void aesni_decrypt(const unsigned char *in, unsigned char *out,
                 const AES_KEY *key);

#endif
