/* IETF Validation tests */
#include <stdio.h>
#include "unabto/unabto_env_base.h"

#if NABTO_ENABLE_TEST_CODE

#include "unabto/unabto_hmac_sha256_test.h"
#include "unabto/unabto_hmac_sha256.h"
#include "modules/crypto/generic/unabto_sha256.h"

bool test(const __ROM char *vector, unsigned char *digest,
          unsigned int digest_size)
{
    char output[2 * UNABTO_SHA256_DIGEST_LENGTH + 1];
    int i;

    output[2 * digest_size] = '\0';

    for (i = 0; i < (int) digest_size ; i++) {
       sprintf(output + 2*i, "%02x", digest[i]);
    }

    NABTO_LOG_INFO(("H: %s", output));
#if __18CXX
    if (strcmpram2pgm(vector, output)) {
#else
    if (strcmp(vector, output)) {
#endif
        NABTO_LOG_INFO(("Test failed."));
        return false;
    }
    return true;
}

    /* HMAC-SHA-256 */
static const __ROM char vector1[] = "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7";
static const __ROM char vector2[] = "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843";
static const __ROM char vector3[] = "773ea91e36800e46854db8ebd09181a72959098b3ef8c122d9635514ced565fe";
static const __ROM char vector4[] = "82558a389a443c0ea4cc819899f2083a85f0faa3e578f8077a2e3ff46729665b";
static const __ROM char vector5[] = "a3b6167473100ee06e0c796c2955552b";
static const __ROM char vector6[] = "60e431591ee0b67f0d8a26aacbf5b77f8e0bc6213728c5140546040f0ee37f54";
static const __ROM char vector7[] = "9b09ffa71b942fcb27635fbcd5b0e944bfdc63644f0713938a7f51535c3a35e2";

static const unsigned char key0[20] = { 0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,
                                                0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b};
static const unsigned char key1[5] = "Jefe"; // only four bytes used!
static const unsigned char key2[20] = {0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
                                               0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa};
static const uint8_t key3[25] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25};
static const unsigned char key4[20] = {0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,
                                               0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c};

static const unsigned char key5[132] = {0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
                                        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
                                        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
                                        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
                                        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
                                        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
                                        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
                                        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
                                        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
                                        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
                                        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
                                        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
                                        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
                                        0xaa, 0x00};
 
//static const __ROM unsigned char key6[131] = key5;


static const unsigned char message3[51] = {0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,
                                  0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,
                                  0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,
                                  0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,
                                  0xdd,0xdd,0x00};
static const unsigned char message4[51] = {0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,
                                  0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,
                                  0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,
                                  0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,
                                  0xcd,0xcd,0x00};

static const unsigned char message1[] = "Hi There";
static const unsigned char message2[] = "what do ya want for nothing?";
static const unsigned char message5[] = "Test With Truncation";
static const unsigned char message6[] = "Test Using Larger Than Block-Size Key - Hash Key First";
static const unsigned char message7[] = "This is a test using a larger than block-size key "
    "and a larger than block-size data. The key needs"
    " to be hashed before being used by the HMAC algorithm.";
const unsigned char* messages[] = {
    message1, message2, message3, message4, message5,message6,message7
};

const __ROM char* vectors[] = {
        vector1, vector2, vector3, vector4, vector5, vector6, vector7
    };

bool hmac_sha256_test(void)
{
    unsigned char mac[UNABTO_SHA256_DIGEST_LENGTH];
    const unsigned char *keys[7];
    unsigned int keys_len[7] = {20, 4, 20, 25, 20, 131, 131};
    int i;
    bool ret = true;
    size_t mac_size;
    size_t msg_len;

    keys[0] = key0;
    keys[1] = key1;
    keys[2] = key2;
    keys[3] = key3;
    keys[4] = key4;
    keys[5] = key5;
    keys[6] = key5;

    NABTO_LOG_INFO(("HMAC-SHA-2 IETF Validation tests"));

#if 0
    unabto_hmac_sha256(keys[0], keys_len[0],
                message1, strlen((const char*)message1), 
                mac, SHA256_DIGEST_SIZE);
    ret &= test(vectors[0], mac, SHA256_DIGEST_SIZE);
#endif

    for (i = 0; i < 7; i++) {
        static uint8_t digest[UNABTO_SHA256_DIGEST_LENGTH];
        static uint8_t digest2[UNABTO_SHA256_DIGEST_LENGTH];
        int r;
        if (i != 4) {
            mac_size = UNABTO_SHA256_DIGEST_LENGTH;
        } else {
            mac_size = 128 / 8;
        }
        NABTO_LOG_TRACE(("Test %d:", i + 1));
        msg_len = strlen((const char*)messages[i]);
        unabto_sha256(messages[i], msg_len, digest);

        unabto_buffer ks[1];
        unabto_buffer ms[1];

        unabto_buffer_init(ks, (uint8_t*)keys[i], keys_len[i]);
        unabto_buffer_init(ms, (uint8_t*)messages[i], msg_len);

        unabto_hmac_sha256_buffers(ks,1, 
                                   ms, 1,
                                   mac, mac_size);
        unabto_sha256(messages[i], msg_len, digest2);
        r = memcmp((const void*)digest,(const void*)digest2,UNABTO_SHA256_DIGEST_LENGTH);
        if (r != 0) {
           NABTO_LOG_TRACE(("The hmac changed the message %i", i));
           ret = false;
           break;
        }
        
        if(!test(vectors[i], mac, mac_size)) {
            ret = false;
            break;
        }
        
    }
    return ret;
}

#else
static char dummy;
#endif
