#ifndef _UNABTO_AES128_SHA256_TEST_H_
#define _UNABTO_AES128_SHA256_TEST_H_

#include <unabto/unabto_env_base.h>

/**
 * test that verification of integrity works as expected.
 */
void truncated_hmac_sha256_verify_integrity_test(bool *a);

int integrity_verify_timing(void);

#endif
