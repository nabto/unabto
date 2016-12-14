#include <stdio.h>
#include "unabto/unabto_env_base.h"

#include "unabto/unabto_external_environment.h"
#include "unabto_sha256_test.h"
#include "modules/crypto/generic/unabto_sha256.h"

void unabto_sha256(const uint8_t* message, size_t message_len, unsigned char* digest);

void unabto_sha256(const uint8_t* message, size_t len, unsigned char *digest)
{
    sha256_ctx ctx;
    
    unabto_sha256_init(&ctx);
    unabto_sha256_update(&ctx, message, len);
    unabto_sha256_final(&ctx, digest);
}

bool sha2_test_test(const __ROM char *vector, unsigned char *digest,
          unsigned int digest_size)
{
    char output[2 * SHA256_DIGEST_SIZE + 1];
    int i;

    output[2 * digest_size] = '\0';

    for (i = 0; i < (int) digest_size ; i++) {
       sprintf(output + 2 * i, "%02x", digest[i]);
    }


#if __18CXX
    if (strcmpram2pgm(vector, output) != 0) {
        NABTO_LOG_INFO(("Test failed. Vec: %HS", vector));
        NABTO_LOG_TRACE(("H: %s", output));
#else
    if (strcmp(vector, output) != 0) {
        NABTO_LOG_INFO(("Test failed. Vec: %s", vector));
        NABTO_LOG_TRACE(("H: %s", output));
#endif
        
        
        return false;
    }

    return true;
}

static const __ROM char vector1[] = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
static const __ROM char vector2[] = "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1";
static const __ROM char vector3[] = "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0";
static const __ROM char vector4[] = "7595af82ae2fa59cd9bf3b4405d31c69b98de71fed5945fd777d8ab3b393a85f";
static const __ROM char vector5[] = "45ad4b37c6e2fc0a2cfcc1b5da524132ec707615c2cae1dbbc43c97aa521db81";
           
static const char message5[] = {
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0x00 };
                      

bool sha256_test(void) {
    static const char message1[] = "abc";
    static const char message2a[] = "abcdbcdecdefdefgefghfghighijhi"
                                    "jkijkljklmklmnlmnomnopnopq";
  /*static const char message2b[] = "abcdefghbcdefghicdefghijdefghijkefghij"
                                    "klfghijklmghijklmnhijklmnoijklmnopjklm"
                                    "nopqklmnopqrlmnopqrsmnopqrstnopqrstu";*/

    static const char message4[] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";


    /* unsigned char *message3; */
    bool ret = true;

    unsigned char digest[SHA256_DIGEST_SIZE];

    NABTO_LOG_INFO(("SHA-2 FIPS 180-2 Validation tests"));

    NABTO_LOG_INFO(("SHA-256 Test vectors"));
//#if 0
    unabto_sha256((const unsigned char *) message1, strlen(message1), digest);
    ret &= sha2_test_test(vector1, digest, SHA256_DIGEST_SIZE);

    unabto_sha256((const unsigned char *) message2a, strlen(message2a), digest);
    ret &= sha2_test_test(vector2, digest, SHA256_DIGEST_SIZE);

    unabto_sha256((const unsigned char *) message4, strlen(message4), digest);
    ret &= sha2_test_test(vector4, digest, SHA256_DIGEST_SIZE);
//#endif
    unabto_sha256((const unsigned char *) message5, strlen(message5), digest);
    ret &= sha2_test_test(vector5, digest, SHA256_DIGEST_SIZE);

//#if 0
/* #ifndef __18CXX */
/*     { */
/*         unsigned int message3_len = 1000000; */
/*         message3 = (unsigned char*)malloc(message3_len); */
/*         if (message3 == NULL) { */
/*             fprintf(stderr, "Can't allocate memory\n"); */
/*             return -1; */
/*         } */
/*         memset(message3, 'a', message3_len); */
/*         sha256(message3, message3_len, digest); */
/*         ret &= sha2_test_test(vector3, digest, SHA256_DIGEST_SIZE); */
/*         free(message3); */
/*     } */
/* #endif */
//#endif

    if (ret) {
        NABTO_LOG_INFO(("sha256 tests succeded"));
    } else {
        NABTO_LOG_INFO(("sha 256 tests failed"));

    }

    return ret;
}

int sha256_timing_test(void) {
    nabto_stamp_t future;
    int i = 0;
    unsigned char digest[SHA256_DIGEST_SIZE];
    size_t len = strlen(message5);
    nabtoSetFutureStamp(&future, 1000);

    while (!nabtoIsStampPassed(&future)) {
        unabto_sha256((const unsigned char *) message5, len, digest);
        i++;
    }
    if (!sha2_test_test(vector5, digest, SHA256_DIGEST_SIZE)) { // sanity check on the last digest calculated
        NABTO_LOG_TRACE(("failure in sha_256 timing test"));
    }
    return i;
}
