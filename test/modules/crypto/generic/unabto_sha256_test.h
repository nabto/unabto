#ifndef _SHA256_TEST_H_
#define _SHA256_TEST_H_

void unabto_sha256(const uint8_t* message, size_t message_len, unsigned char* digest);

bool sha256_test(void);

int sha256_timing_test(void);

#endif
