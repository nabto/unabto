#include "unabto/unabto_env_base.h"
#include "unabto/unabto_util.h"
#include "unabto_aes_test.h"
#include "modules/crypto/generic/unabto_aes.h"
#include "unabto/unabto_external_environment.h"

bool aes128_test(const uint8_t key[16], uint8_t plaintext[16],
                 const uint8_t result[16]) {
  
    uint32_t block[4];

    AES_CTX ctx;

    int i;

    AES_set_key(&ctx, key, key, AES_MODE_128);
    for (i = 0; i < 4; i++) {
        WRITE_U32(&block[i], *(uint32_t*)(plaintext+(i*sizeof(uint32_t))));
    }


    AES_encrypt(&ctx, block);

    for (i = 0; i < 4; i++) {
        uint32_t input = block[i];
        uint8_t* output = plaintext+(i*sizeof(uint32_t));
        WRITE_U32(output, input);
    }
    
    return (memcmp((const uint8_t*)plaintext, result, 16) == 0);
}

bool aes_test() {
    const uint8_t key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 
                            0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };

    uint8_t plaintext[] = {
        0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 
        0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a };

    const uint8_t result[] = {
        0x3a, 0xd7, 0x7b, 0xb4, 0x0d, 0x7a, 0x36, 0x60, 
        0xa8, 0x9e, 0xca, 0xf3, 0x24, 0x66, 0xef, 0x97 };
    return aes128_test(key, plaintext, result);
}

int aes_timing_test(void) {
    nabto_stamp_t future;
    int i = 0;
    bool r = true;
    nabtoSetFutureStamp(&future, 1000);
    while (!nabtoIsStampPassed(&future)) {
        r &= aes_test();
        i++;
    }
    if (!r) {
        NABTO_LOG_TRACE(("failure in aes timing test"));
    }
    return i;
}
