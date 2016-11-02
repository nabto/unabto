#include "read_hex.h"

#include <stdio.h>

#if defined(__STDC__)
# if defined(__STDC_VERSION__)
#  if (__STDC_VERSION__ >= 199901L)
#   define C99
#  endif
# endif
#endif

size_t read_hex(const char* string, size_t stringLength, uint8_t* buffer, size_t bufferLength)
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
