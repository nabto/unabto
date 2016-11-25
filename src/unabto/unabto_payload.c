#include "unabto_payload.h"
#include "unabto_util.h"


uint8_t* unabto_read_payload(uint8_t* begin, uint8_t* end, struct unabto_payload_packet* payload)
{
    ptrdiff_t bufferLength = end - begin;
    uint8_t* ptr = begin;
    if (bufferLength == 0) {
        // end reached expected
        return NULL;
    }
    if (bufferLength < 4) {
        NABTO_LOG_ERROR(("unexpected end of payloads"));
        return NULL;
    }
    READ_FORWARD_U8(payload->type, ptr);
    READ_FORWARD_U8(payload->flags, ptr);
    READ_FORWARD_U16(payload->length, ptr);

    // check that the payload length is not greater than the buffer.
    if (payload->length > bufferLength) {
        return NULL;
    }

    payload->dataBegin = ptr;
    payload->dataEnd = begin + payload->length;
    return payload->dataEnd;
}
