#include <stdio.h>

#include <modules/crypto/openssl_minimal/unabto_openssl_minimal_aes.h>
#include <modules/crypto/openssl_minimal/unabto_openssl_minimal_sha256.h>
#include <unabto/unabto_util.h>

bool aes128_test(const uint8_t key[16], const uint8_t plaintext[16],
                 const uint8_t result[16]) {
  
    uint8_t encrypted[16];

    AES_KEY ctx;

    int i;

    private_AES_set_encrypt_key(key, 128, &ctx);
    
    AES_encrypt(plaintext, encrypted, &ctx);

    return (memcmp((const uint8_t*)encrypted, result, 16) == 0);
}

bool aes_test() {
    const uint8_t key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 
                            0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };

    const uint8_t plaintext[] = {
        0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 
        0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a };

    const uint8_t result[] = {
        0x3a, 0xd7, 0x7b, 0xb4, 0x0d, 0x7a, 0x36, 0x60, 
        0xa8, 0x9e, 0xca, 0xf3, 0x24, 0x66, 0xef, 0x97 };
    return aes128_test(key, plaintext, result);
}

static const uint8_t vector1[] = { 0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23, 0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c, 0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad };
static const char message1[] = "abc";

bool sha256_test() {
    SHA256_CTX ctx;
    uint8_t digest[SHA256_DIGEST_LENGTH];
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, message1, strlen(message1));
    SHA256_Final(digest, &ctx);

    return (memcmp(digest, vector1, SHA256_DIGEST_LENGTH) == 0);
}


int main(int argc, char** argv) {
    if (!aes_test()) {
        printf("aes test failed\n");
        exit(1);
    }

    if (!sha256_test()) {
        printf("sha256 test failed\n");
        exit(1);
    }
    printf("all tests passed\n");
}
