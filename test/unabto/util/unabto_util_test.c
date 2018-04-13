
#include "unabto/unabto_env_base.h"
#include "unabto/unabto_util.h"

bool unabto_util_test(void) {

    uint8_t buf[] = {0x00,0x00,0x00,0x00,0x12, 0x34, 0x56, 0x78};
    uint32_t* buf2 = (uint32_t*)buf;
    uint8_t buf3[sizeof(buf)];
    uint8_t* pbuf;
    uint32_t testNr;
    uint16_t testu16;
    bool ret = true;
    READ_U32(testNr, buf+4);
    if (testNr != 305419896) {       // 0x12345678 in host byte order
        NABTO_LOG_TRACE(("testNr: %i", testNr));
        ret = false;
    }
    
    READ_U32(testNr, buf2+1);
    if (testNr != 0x12345678) {
        NABTO_LOG_TRACE(("testNr: %i", testNr));
        ret = false;
    }

    READ_U16(testu16, buf+4);
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

    pbuf = buf+4;
    READ_FORWARD_MEM(buf3, pbuf, 2);
    if (buf3[0] != 0x12 ||
        buf3[1] != 0x34) {
        NABTO_LOG_ERROR(("READ_FORWARD_MEM failed"));
        ret = false;
    }
    READ_FORWARD_MEM(buf3, pbuf, 2);
    if (buf3[0] != 0x56 ||
        buf3[1] != 0x78) {
        NABTO_LOG_ERROR(("READ_FORWARD_MEM 2nd read failed"));
        ret = false;
    }

    return ret;
    
}
