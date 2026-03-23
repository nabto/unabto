
#include "unabto/unabto_env_base.h"
#include "unabto/unabto_util.h"

bool unabto_util_test(void) {
    uint8_t buf[] = {0x00, 0x00, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78};
    uint32_t* buf2 = (uint32_t*)buf;
    uint8_t buf3[sizeof(buf)];
    uint32_t testNr;
    uint16_t testu16;
    bool ret = true;
    READ_U32(testNr, buf + 4);
    if (testNr != 305419896) {  // 0x12345678 in host byte order
        NABTO_LOG_TRACE(("testNr: %i", testNr));
        ret = false;
    }

    READ_U32(testNr, buf2 + 1);
    if (testNr != 0x12345678) {
        NABTO_LOG_TRACE(("testNr: %i", testNr));
        ret = false;
    }

    READ_U16(testu16, buf + 4);
    if (testu16 != 4660) {
        NABTO_LOG_TRACE(("testu16 read failed %i", testu16));
        ret = false;
    }

    WRITE_U32(buf, 0x01020304);
    if (buf[0] != 0x01 ||
        buf[1] != 0x02 ||
        buf[2] != 0x03 ||
        buf[3] != 0x04) {
        NABTO_LOG_TRACE(("WRITE u32 failed"));
        ret = false;
    }

    WRITE_U16(buf, 0x0506);
    if (buf[0] != 0x05 ||
        buf[1] != 0x06) {
        NABTO_LOG_TRACE(("WRITE u16 failed"));
        ret = false;
    }

    {
        const uint8_t* rfm_end = buf + sizeof(buf);
        const uint8_t* rfm_p = buf + 4;
        rfm_p = read_forward_mem(buf3, rfm_p, rfm_end, 2);
        if (rfm_p == NULL || buf3[0] != 0x12 || buf3[1] != 0x34) {
            NABTO_LOG_ERROR(("read_forward_mem failed"));
            ret = false;
        }
        rfm_p = read_forward_mem(buf3, rfm_p, rfm_end, 2);
        if (rfm_p == NULL || buf3[0] != 0x56 || buf3[1] != 0x78) {
            NABTO_LOG_ERROR(("read_forward_mem 2nd read failed"));
            ret = false;
        }
    }

    /* Tests for bounds-checked read_forward_* functions */
    {
        uint8_t data[] = {0xAB, 0x00, 0x12, 0x12, 0x34, 0x56, 0x78, 0xDE};
        const uint8_t* end = data + sizeof(data);
        const uint8_t* p;
        uint8_t u8val;
        uint16_t u16val;
        uint32_t u32val;
        uint8_t membuf[4];

        /* Normal read_forward_u8 */
        p = read_forward_u8(&u8val, data, end);
        if (p == NULL || u8val != 0xAB) {
            NABTO_LOG_ERROR(("read_forward_u8 normal failed"));
            ret = false;
        }

        /* Normal read_forward_u16 */
        p = read_forward_u16(&u16val, data + 3, end);
        if (p == NULL || u16val != 0x1234) {
            NABTO_LOG_ERROR(("read_forward_u16 normal failed"));
            ret = false;
        }

        /* Normal read_forward_u32 */
        p = read_forward_u32(&u32val, data + 3, end);
        if (p == NULL || u32val != 0x12345678) {
            NABTO_LOG_ERROR(("read_forward_u32 normal failed"));
            ret = false;
        }

        /* Normal read_forward_mem */
        p = read_forward_mem(membuf, data + 3, end, 4);
        if (p == NULL || membuf[0] != 0x12 || membuf[1] != 0x34 || membuf[2] != 0x56 || membuf[3] != 0x78) {
            NABTO_LOG_ERROR(("read_forward_mem normal failed"));
            ret = false;
        }

        /* Exact boundary: read_forward_u8 at last byte */
        p = read_forward_u8(&u8val, data + 7, end);
        if (p == NULL || u8val != 0xDE) {
            NABTO_LOG_ERROR(("read_forward_u8 exact boundary failed"));
            ret = false;
        }

        /* Exact boundary: read_forward_u32 at last 4 bytes */
        /* data+4 = {0x34, 0x56, 0x78, 0xDE} -> 0x345678DE */
        p = read_forward_u32(&u32val, data + 4, end);
        if (p == NULL || u32val != 0x345678DE) {
            NABTO_LOG_ERROR(("read_forward_u32 exact boundary failed"));
            ret = false;
        }

        /* Past end: read_forward_u8 past buffer */
        p = read_forward_u8(&u8val, end, end);
        if (p != NULL) {
            NABTO_LOG_ERROR(("read_forward_u8 past end should return NULL"));
            ret = false;
        }

        /* Past end: read_forward_u16 with only 1 byte left */
        p = read_forward_u16(&u16val, data + 7, end);
        if (p != NULL) {
            NABTO_LOG_ERROR(("read_forward_u16 past end should return NULL"));
            ret = false;
        }

        /* Past end: read_forward_u32 with only 3 bytes left */
        p = read_forward_u32(&u32val, data + 5, end);
        if (p != NULL) {
            NABTO_LOG_ERROR(("read_forward_u32 past end should return NULL"));
            ret = false;
        }

        /* Past end: read_forward_mem with insufficient space */
        p = read_forward_mem(membuf, data + 6, end, 4);
        if (p != NULL) {
            NABTO_LOG_ERROR(("read_forward_mem past end should return NULL"));
            ret = false;
        }

        /* NULL input propagation */
        p = read_forward_u8(&u8val, NULL, end);
        if (p != NULL) {
            NABTO_LOG_ERROR(("read_forward_u8 NULL propagation failed"));
            ret = false;
        }

        p = read_forward_u16(&u16val, NULL, end);
        if (p != NULL) {
            NABTO_LOG_ERROR(("read_forward_u16 NULL propagation failed"));
            ret = false;
        }

        p = read_forward_u32(&u32val, NULL, end);
        if (p != NULL) {
            NABTO_LOG_ERROR(("read_forward_u32 NULL propagation failed"));
            ret = false;
        }

        p = read_forward_mem(membuf, NULL, end, 2);
        if (p != NULL) {
            NABTO_LOG_ERROR(("read_forward_mem NULL propagation failed"));
            ret = false;
        }
    }

    return ret;
}
