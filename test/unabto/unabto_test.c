#define NABTO_DECLARED_MODULE NABTO_LOG_APP

#include "unabto/unabto_env_base.h"

#if NABTO_ENABLE_TEST_CODE

#include "unabto_test.h"

#include "modules/crypto/generic/unabto_sha256_test.h"
#include "modules/crypto/generic/unabto_aes_test.h"
#include "unabto_hmac_sha256_test.h"
#include "unabto_aes_cbc_test.h"
#include "unabto_prfplus_test.h"
#include "unabto_aes128_sha256_test.h"
#include "util/unabto_util_test.h"
#include "util/unabto_buffer_test.h"
#include "unabto_stream_event_test.h"
#include "modules/util/unabto_base32_test.h"
#include <unabto/unabto_stream_event_test.h>

/** Enable SHA256 test by default */
#ifndef NABTO_ENABLE_SHA256_TESTS
#define NABTO_ENABLE_SHA256_TESTS 1 
#endif

bool unabto_test_all(void) {
//    NABTO_DECLARE_LOCAL_MODULE(NABTO_LOG_APP);
    bool ret = true;
    bool r;

    NABTO_LOG_INFO(("Testing sha256 implementation"));

    r = sha256_test();
    if (!r) {
        NABTO_LOG_INFO(("sha256 test failed"));
        ret = false;
    }


#if NABTO_ENABLE_SHA256_TESTS
    NABTO_LOG_INFO(("Testing hmac_sha256"));
    r = hmac_sha256_test();
    if (!r) {
        NABTO_LOG_INFO(("hmac sha256 tests failed"));
        ret = false;
    }
#endif

    NABTO_LOG_INFO(("Testing AES implementation"));
    r = aes_test();
    if (!r) {
        NABTO_LOG_INFO(("AES test failed"));
        ret = false;
    }

    NABTO_LOG_INFO(("Testing AES cbc encrypt/decrypt"));
    r = aes_cbc_test();
    if (!r) {
        NABTO_LOG_INFO(("AES_CBC encrypt/decrypt failed"));
        ret = false;
    }

    NABTO_LOG_INFO(("Testing prfplus"));
    r = unabto_prfplus_test();
    if (!r) {
        NABTO_LOG_INFO(("Prfplus_sha256 failed"));
        ret = false;
    }

    NABTO_LOG_INFO(("Testing truncated_hmac_sha256_verify_integrity"));
    truncated_hmac_sha256_verify_integrity_test(&r);
    //NABTO_LOG_TRACE(("ret %u", r));
    if (!r) {
        NABTO_LOG_INFO(("integrity verification test failed"));
        ret = false;
    }

    NABTO_LOG_INFO(("testing unabto_crypto.c"));
    r = test_nabto_crypto_create_key_material();
    if (!r) {
        NABTO_LOG_INFO(("unabto_crypto.c test failed"));
        ret = false;
    }

    NABTO_LOG_INFO(("Testing unabto_util"));
    r = unabto_util_test();
    if (!r) {
        NABTO_LOG_INFO(("testing of unabto_util failed"));
        ret = false;
    }

    NABTO_LOG_INFO(("Testing unabto_buffer"));
    r = unabto_buffer_test();
    if (!r) {
        NABTO_LOG_ERROR(("Test of unabto_buffer failed"));
        ret = false;
    }

    {
        int i;
        NABTO_LOG_INFO(("testing unabto crypto timings"));
        i = integrity_verify_timing();
        NABTO_LOG_INFO(("%i integrity checks in one second.", i));

        i = aes_cbc_timing_test();
        NABTO_LOG_INFO(("%i aes_cbc en/decryptions in one second.", i));

        i = sha256_timing_test();
        NABTO_LOG_INFO(("%i 131bytes sha256 hashes in one second.", i));

        i = aes_timing_test();
        NABTO_LOG_INFO(("%i aes blocks en/decryptiong in one second.", i));
    }

    r = test_state_machine();
    
    if (!r) {
        NABTO_LOG_ERROR(("Test of unabto stream state machine failed."));
        ret = false;
    }

    r = unabto_base32_test();
    if (!r) {
        NABTO_LOG_ERROR(("Test of base32 failed."));
        ret = false;
    }
    
    if (ret) {
        NABTO_LOG_INFO(("All uNabto tests succeded"));
    } else {
        NABTO_LOG_INFO(("Some uNabto test failed"));
    }
    return ret;
}

#endif
