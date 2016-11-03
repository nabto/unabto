#include "read_hex.h"

#include <unabto/unabto_logging.h>

#include <stdio.h>

#if defined(__STDC__)
# if defined(__STDC_VERSION__)
#  if (__STDC_VERSION__ >= 199901L)
#   define C99
#  endif
# endif
#endif

size_t unabto_read_hex(const char* string, size_t stringLength, uint8_t* buffer, size_t bufferLength)
{
    int i = 0;
    for (i = 0; i < stringLength/2 && i < bufferLength; i++) {
#if defined(C99)
        uint8_t byte;
#else        
        unsigned int byte;
#endif
        sscanf(string+(2*i), "%02hhx", &byte);
        buffer[i] = (uint8_t)byte;
    }
    return i;
}

bool unabto_read_psk_from_hex(const char* preSharedKey, uint8_t* psk, size_t pskLength)
{
    size_t inputLength = strlen(preSharedKey);
    if (inputLength != 32) {
        NABTO_LOG_ERROR(("The pre shared key should be 32 hex characters long"));
        return false;
    }

    if (pskLength != 16) {
        NABTO_LOG_ERROR(("The pre shared key buffer needs to be exactly 16 bytes"));
        return false;
    }
    if (pskLength != unabto_read_hex(preSharedKey, inputLength, psk, pskLength)) {
        NABTO_LOG_ERROR(("Failed to read %" PRIsize " bytes from the pre shared key", pskLength));
        return false;
    }
    return true;
}
