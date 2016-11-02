#include <unabto_platform_types.h>

/**
 * Read hex from the input string store it in the buffer and return the number of bytes stored
 */
size_t read_hex(const char* string, size_t stringLength, uint8_t* buffer, size_t bufferLength);
