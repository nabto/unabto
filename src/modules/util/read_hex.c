#include "read_hex.h"

#include <unabto/unabto_env_base.h>
#include <unabto/unabto_logging.h>

/**
 * sscanf is hard to get right across platforms since c89 and c99 do
 * it differently
 */
bool convert_from_hex(char c, uint8_t* byte)
{
    switch (c) {
        case '0': *byte = 0; break;
        case '1': *byte = 1; break;
        case '2': *byte = 2; break;
        case '3': *byte = 3; break;
        case '4': *byte = 4; break;
        case '5': *byte = 5; break;
        case '6': *byte = 6; break;
        case '7': *byte = 7; break;
        case '8': *byte = 8; break;
        case '9': *byte = 9; break;
        case 'a': case 'A': *byte = 10; break;
        case 'b': case 'B': *byte = 11; break;
        case 'c': case 'C': *byte = 12; break;
        case 'd': case 'D': *byte = 13; break;
        case 'e': case 'E': *byte = 14; break;
        case 'f': case 'F': *byte = 15; break;
        default: return false;
    }
    return true;
}

bool unabto_read_hex(const char* string, size_t stringLength, uint8_t* buffer, size_t bufferLength, size_t* outLength)
{
    size_t i = 0;
    *outLength = 0;
    for (i = 0; i < stringLength/2 && i < bufferLength; i++) {
        uint8_t b1, b2;
        if (convert_from_hex(string[2*i], &b1) && convert_from_hex(string[(2*i)+1], &b2)) {
            uint8_t byte = (b1 << 4) | b2;
            buffer[i] = byte;
        } else {
            return false;
        }
        *outLength = i+1;
    }
    return true;
}

bool unabto_read_psk_from_hex(const char* preSharedKey, uint8_t* psk, size_t pskLength)
{
    size_t inputLength = strlen(preSharedKey);
    size_t outLength = 0;
    if (inputLength != 32) {
        NABTO_LOG_ERROR(("The pre shared key should be 32 hex characters long"));
        return false;
    }

    if (pskLength != 16) {
        NABTO_LOG_ERROR(("The pre shared key buffer needs to be exactly 16 bytes"));
        return false;
    }
    if (!unabto_read_hex(preSharedKey, inputLength, psk, pskLength, &outLength)) {
        NABTO_LOG_ERROR(("Failed to read %" PRIsize " bytes from the pre shared key error at position %" PRIsize, pskLength, outLength));
        return false;
    }
    
    return true;
}
