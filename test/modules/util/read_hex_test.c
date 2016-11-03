#include <unabto/unabto_env_base.h>
#include <modules/util/read_hex.h>
#include <unabto/unabto_logging.h>

bool hex_test(const char* string, uint8_t* expected, size_t expectedLength);

bool read_hex_test(void)
{
    uint8_t expected1[8] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef };
    uint8_t expected2[0] = {};

    NABTO_LOG_INFO(("read hex test"));
    
    return
        hex_test("0123456789abcdef", expected1, 8) &&
        hex_test("0123456789abcdef0", expected1, 8) &&
        hex_test("fgh3456789abcdef", expected2, 0);
}

#define MAX_BUFFER_LENGTH 42

bool hex_test(const char* string, uint8_t* expected, size_t expectedLength)
{
    uint8_t buffer[MAX_BUFFER_LENGTH];
    if (expectedLength == unabto_read_hex(string, strlen(string), buffer, MAX_BUFFER_LENGTH)) {
        if (memcmp(expected,buffer, expectedLength) == 0) {
            return true;
        } else {
            NABTO_LOG_ERROR(("invalid read hex"));
        }
    }
    return false;
}
