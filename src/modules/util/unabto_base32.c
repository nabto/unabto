#include "unabto_base32.h"
#include <unabto/unabto_env_base.h>
#include <unabto/unabto_logging.h>

static const char base32_alphabet[] = "abcdefghijklmnopqrstuvwxyz234567";
uint8_t base32_to_bits(char value)
{
    if (value >= 'a' && value <= 'z') {
        return (value-'a');
    }

    if (value >= 'A' && value <= 'Z') {
        return (value-'A');
    }
    
    if (value >= '2' && value <= '7') {
        return 26+(value-'2');
    }
    NABTO_LOG_ERROR(("invalid base32 character %i", value));
    return 0;
}

uint8_t* unabto_base32_encode(uint8_t* outputBuffer, uint8_t* outputBufferEnd, uint8_t* inputBuffer, uint8_t* inputBufferEnd)
{
    uint16_t buffer = 0;
    uint8_t bufferBits = 0;
    while(inputBuffer != inputBufferEnd) {
        buffer <<= 8;
        buffer |= *inputBuffer;
        inputBuffer++;
        bufferBits += 8;
        while (bufferBits >= 5) {
            uint16_t top = (buffer >> (bufferBits - 5)) & 0x1f;
            *outputBuffer = base32_alphabet[top];
            outputBuffer++;
            bufferBits -= 5;
        }
    }
    if (bufferBits > 0) {
        uint16_t top;
        buffer <<= 8;
        bufferBits += 8;
        top = (buffer >> (bufferBits - 5)) & 0x1f;
        *outputBuffer = base32_alphabet[top];
        outputBuffer++;
        bufferBits -= 5;
    }
    return outputBuffer;
}
    

uint8_t* unabto_base32_decode(uint8_t* outputBuffer, uint8_t* outputBufferEnd, uint8_t* inputBuffer, uint8_t* inputBufferEnd)
{
    uint16_t buffer = 0;
    uint8_t bufferBits = 0;
    
    while (inputBuffer != inputBufferEnd) {
        buffer <<= 5;
        buffer |= base32_to_bits(*inputBuffer);
        bufferBits += 5;
        inputBuffer++;
        if (bufferBits >= 8) {
            *outputBuffer = (buffer >> (bufferBits - 8)) & 0xff;
            outputBuffer++;
            bufferBits -= 8;
        }
    }
    return outputBuffer;
}
