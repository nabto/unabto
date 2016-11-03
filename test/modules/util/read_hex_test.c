#include <unabto/unabto_env_base.h>
#include <modules/util/read_hex.h>
#include <unabto/unabto_logging.h>

bool hex_test(const char* string, uint8_t* expected, size_t expectedLength);
bool negative_hex_test(const char* string);

bool read_hex_test(void)
{
    uint8_t expected1[8] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef };
    uint8_t expected2[0] = {};

    NABTO_LOG_INFO(("read hex test"));
    
    return
        hex_test("", expected2, 0) &&
        hex_test("0123456789abcdef", expected1, 8) &&
        hex_test("0123456789abcdef0", expected1, 8) &&
        negative_hex_test("fgh3456789abcdef");
}

#define MAX_BUFFER_LENGTH 42

bool hex_test(const char* string, uint8_t* expected, size_t expectedLength)
{
    uint8_t buffer[MAX_BUFFER_LENGTH];
    size_t readLength;
    if (unabto_read_hex(string, strlen(string), buffer, MAX_BUFFER_LENGTH, &readLength)) {
        if (expectedLength == readLength && memcmp(expected,buffer, expectedLength) == 0) {
            return true;
        } else {
            NABTO_LOG_ERROR(("invalid read hex in string %s at position %" PRIsize, string, readLength));
        }
    } else {
        NABTO_LOG_ERROR(("read hex for %s failed", string));
    }
    return false;
}

bool negative_hex_test(const char* string) {
    uint8_t buffer[MAX_BUFFER_LENGTH];
    size_t readLength;
    if (unabto_read_hex(string, strlen(string), buffer, MAX_BUFFER_LENGTH, &readLength)) {
        NABTO_LOG_ERROR(("didn't expect to be able to read hex from %s", string));
        return false;
    } else {
        return true;
    }
}
