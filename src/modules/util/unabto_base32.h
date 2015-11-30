#ifndef _UNABTO_BASE32_H_
#define _UNABTO_BASE32_H_

#include <unabto_platform_types.h>

uint8_t* unabto_base32_encode(uint8_t* outputBuffer, uint8_t* outputBufferEnd, uint8_t* inputBuffer, uint8_t* inputBufferEnd);

uint8_t* unabto_base32_decode(uint8_t* outputBuffer, uint8_t* outputBufferEnd, uint8_t* inputBuffer, uint8_t* inputBufferEnd);

#endif
