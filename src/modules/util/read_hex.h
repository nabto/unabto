#include <unabto_platform_types.h>

/**
 * Read hex from the input string store it in the buffer and return the number of bytes stored
 */
bool unabto_read_hex(const char* string, size_t stringLength, uint8_t* buffer, size_t bufferLength, size_t* outLength);

bool unabto_read_psk_from_hex(const char* string, uint8_t* psk, size_t pskLength);
