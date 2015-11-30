#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_TEST

#include <unabto/unabto_env_base.h>

#if NABTO_ENABLE_TEST_CODE

#include <unabto/unabto_util.h>
#include <modules/crypto/generic/unabto_aes.h>
#include <unabto/unabto_external_environment.h>
#include <../external/microchip_del/AES_ASM.h>

static bool aes128_encode_test(const uint8_t __key[16], const uint8_t plaintext[16], const uint8_t result[16]);
static bool aes128_decode_test(const uint8_t __key[16], const uint8_t plaintext[16], const uint8_t result[16]);

static const uint8_t testKey[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
static const uint8_t testPlaintext[] = {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a};
static const uint8_t testResult[] = {0x3a, 0xd7, 0x7b, 0xb4, 0x0d, 0x7a, 0x36, 0x60, 0xa8, 0x9e, 0xca, 0xf3, 0x24, 0x66, 0xef, 0x97};

bool aes_test(void)
{
  if(aes128_encode_test(testKey, testPlaintext, testResult) == false)
  {
    NABTO_LOG_TRACE(("aes128_encode_test() failed!"));
    return false;
  }

  if(aes128_decode_test(testKey, testPlaintext, testResult) == false)
  {
    NABTO_LOG_TRACE(("aes128_decode_test() failed!"));
    return false;
  }

  return true;
}

int aes_timing_test(void)
{
  nabto_stamp_t future;
  int i = 0;

  nabtoSetFutureStamp(&future, 1000);

  while(!nabtoIsStampPassed(&future))
  {
    aes_test();
    i++;
  }

  return i;
}

static bool aes128_encode_test(const uint8_t __key[16], const uint8_t plaintext[16], const uint8_t result[16])
{
  memcpy(block, plaintext, 16);
  memcpy(key, __key, 16);
  AESEncrypt();

  return memcmp((const uint8_t*) block, result, 16) == 0;
}

static bool aes128_decode_test(const uint8_t __key[16], const uint8_t plaintext[16], const uint8_t result[16])
{
  uint8_t keyTemp[16];

  memcpy(key, __key, 16);
  AESCalcDecryptKey();

  memcpy(block, result, 16);
  AESDecrypt();

  return memcmp((const uint8_t*) block, plaintext, 16) == 0;
}

#endif
