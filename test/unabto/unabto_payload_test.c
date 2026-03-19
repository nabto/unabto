#include "unabto_payload_test.h"
#include <unabto/unabto_packet_util.h>
#include <unabto/unabto_util.h>

#include <string.h>

int test_read_payload(void)
{
    uint8_t buffer[42];
    uint8_t* ptr = buffer;
    const uint8_t* end = buffer + sizeof(buffer);
    ptr = write_forward_u8(ptr, end, 42);
    ptr = write_forward_u8(ptr, end, 42);
    ptr = write_forward_u16(ptr, end, 6);
    ptr = write_forward_u16(ptr, end, 42);
    struct unabto_payload_packet payload;
    const uint8_t* result = unabto_read_payload(buffer, buffer+42, &payload);

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

int test_insert_header_bounds(void)
{
    /* insert_header needs at least NP_PACKET_HDR_MIN_BYTELENGTH (16) bytes */
    uint8_t small[10];
    uint8_t exact[NP_PACKET_HDR_MIN_BYTELENGTH];
    uint8_t* res;

    /* Buffer too small -> NULL */
    res = insert_header(small, small + sizeof(small), 0, 0, 0, false, 0, 0, NULL);
    if (res != NULL) return false;

    /* Exact size buffer -> success */
    memset(exact, 0, sizeof(exact));
    res = insert_header(exact, exact + sizeof(exact), 1, 2, 3, false, 4, 0, NULL);
    if (res == NULL) return false;
    if (res != exact + NP_PACKET_HDR_MIN_BYTELENGTH) return false;

    /* NULL buf -> NULL */
    res = insert_header(NULL, exact + sizeof(exact), 0, 0, 0, false, 0, 0, NULL);
    if (res != NULL) return false;

    /* With nsico, buffer too small for the extra 8 bytes */
    {
        uint8_t nsico[8] = {1,2,3,4,5,6,7,8};
        uint8_t buf_nsico[NP_PACKET_HDR_MIN_BYTELENGTH + 4]; /* 4 bytes short */
        res = insert_header(buf_nsico, buf_nsico + sizeof(buf_nsico), 0, 0, 0, false, 0, 0, nsico);
        if (res != NULL) return false;

        /* With nsico, exact size */
        {
            uint8_t buf_ok[NP_PACKET_HDR_MIN_BYTELENGTH + 8];
            res = insert_header(buf_ok, buf_ok + sizeof(buf_ok), 0, 0, 0, false, 0, 0, nsico);
            if (res == NULL) return false;
            if (res != buf_ok + NP_PACKET_HDR_MIN_BYTELENGTH + 8) return false;
        }
    }

    return true;
}

int test_insert_optional_payload_null(void)
{
    uint8_t buf[64];
    uint8_t* res;

    /* NULL input -> NULL output */
    res = insert_optional_payload(NULL, buf + sizeof(buf), 1, NULL, 0);
    if (res != NULL) return false;

    /* Normal case should succeed */
    res = insert_optional_payload(buf, buf + sizeof(buf), 1, NULL, 0);
    if (res == NULL) return false;

    /* Buffer too small -> NULL */
    res = insert_optional_payload(buf, buf + 2, 1, NULL, 0);
    if (res != NULL) return false;

    return true;
}

int test_insert_length_bounds(void)
{
    uint8_t buf[NP_PACKET_HDR_MIN_BYTELENGTH];
    bool ok;

    /* Normal case */
    ok = insert_packet_length(buf, buf + sizeof(buf), 42);
    if (!ok) return false;

    /* Buffer too short */
    ok = insert_packet_length(buf, buf + OFS_PACKET_LGT + 1, 42);
    if (ok) return false;

    /* NULL buf */
    ok = insert_packet_length(NULL, buf + sizeof(buf), 42);
    if (ok) return false;

    return true;
}

int test_insert_length_from_cursor(void)
{
    uint8_t buf[NP_PACKET_HDR_MIN_BYTELENGTH + 10];
    bool ok;

    /* Normal case: packetEnd within buffer */
    ok = insert_packet_length_from_cursor(buf, buf + 20);
    if (!ok) return false;

    /* NULL packetEnd */
    ok = insert_packet_length_from_cursor(buf, NULL);
    if (ok) return false;

    /* NULL packetBegin */
    ok = insert_packet_length_from_cursor(NULL, buf + 10);
    if (ok) return false;

    /* packetEnd before packetBegin */
    ok = insert_packet_length_from_cursor(buf + 5, buf);
    if (ok) return false;

    return true;
}
