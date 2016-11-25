#include "unabto_payload_test.h"
#include <unabto/unabto_packet_util.h>
#include <unabto/unabto_util.h>

int test_read_payload(void)
{
    uint8_t buffer[42];
    uint8_t* ptr = buffer;
    WRITE_FORWARD_U8(ptr, 42);
    WRITE_FORWARD_U8(ptr, 42);
    WRITE_FORWARD_U16(ptr, 6);
    WRITE_FORWARD_U16(ptr, 42);
    struct unabto_payload_packet payload;
    uint8_t* result = unabto_read_payload(buffer, buffer+42, &payload);

    if (result == NULL) {
        return false;
    }

    if ((payload.type != 42) ||
        (payload.flags != 42) ||
        (payload.length != 6) ||
        (payload.dataBegin != buffer+4) ||
        (payload.dataEnd != buffer+6))
    {
        return false;
    }
    return true;
}
