#include "unabto_base32_test.h"

#include <modules/util/unabto_base32.h>
#include <unabto/unabto_util.h>

bool unabto_base32_test(void)
{
    NABTO_LOG_INFO(("base32 encode test"));
    bool status = true;
    uint8_t input[7];
    uint8_t* ptr = input;
    WRITE_FORWARD_U8(ptr, 0x6f);
    WRITE_FORWARD_U16(ptr, 0x0007);
    WRITE_FORWARD_U16(ptr, 0);
    WRITE_FORWARD_U16(ptr, 0x4242);

    uint8_t base32output[12];
    uint8_t* end = unabto_base32_encode(base32output, base32output+12, input, input+7);

    uint8_t result[12] = { 'n', '4', 'a', 'a', 'o', 'a', 'a', 'a', 'i', 'j', 'b', 'a' };
    if (end != base32output+12) {
        NABTO_LOG_INFO(("invalid length"));
        status = false;
    }

    if (memcmp(base32output, result, 12) != 0) {
        NABTO_LOG_INFO(("invalid base32 encoding"));
        status = false;
    }

    uint8_t decoded[7];
    end = unabto_base32_decode(decoded, decoded+7, base32output, base32output+12);
    
    if (memcmp(decoded, input, 7) != 0) {
        NABTO_LOG_INFO(("base32 decode failed."));
        status = false;
    }

    if (end - decoded != 7) {
        NABTO_LOG_INFO(("base32 decode failed, invalid length"));
        status = false;
    }

    return status;
}
