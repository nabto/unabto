#include "unabto_buffer_test.h"

#include "unabto/util/unabto_buffer.h"

bool unabto_buffer_test(void)
{
    char * raw_test_string = "asjdhc#21?(!?(92814skzjbcasa";

    int data_length = 7 + 2 + strlen(raw_test_string);
    uint8_t* data = (uint8_t*)malloc(data_length);
    unabto_buffer buf, raw_string_buf;
    buffer_write_t w_buf;
    buffer_read_t r_buf;

    uint32_t t32;
    uint16_t t16;
    uint8_t  t8;
    int raw_string_data_length = strlen(raw_test_string) + 1;
    uint8_t* raw_string_data = (uint8_t*)malloc(raw_string_data_length);

    buffer_init(&raw_string_buf, (uint8_t*)raw_test_string, strlen(raw_test_string));
    
    buffer_init(&buf, data, data_length);
    
    buffer_write_init(&w_buf, &buf);

    if (! (buffer_write_uint32(&w_buf, 0x12345678) &&
           buffer_write_uint16(&w_buf, 0x1234) &&
           buffer_write_uint8(&w_buf, 0x12) && 
           buffer_write_raw(&w_buf, &raw_string_buf) ) ) {
        NABTO_LOG_ERROR(("Buffer write test failed"));
        return false;
    }
    
    buffer_read_init(&r_buf, &buf);
    
    memset(raw_string_data, 0, raw_string_data_length);
    buffer_init(&raw_string_buf, raw_string_data, raw_string_data_length);
    bool t = false;    
    if (! ( buffer_read_uint32(&r_buf, &t32) && t32 == 0x12345678 &&
            buffer_read_uint16(&r_buf, &t16) && t16 == 0x1234 &&
            buffer_read_uint8(&r_buf, &t8)   && t8  == 0x12 && 
            (t = buffer_read_raw(&r_buf, &raw_string_buf)) && buffer_get_size(&raw_string_buf) == strlen(raw_test_string) &&
            0 == strncmp((char*)buffer_get_data(&raw_string_buf), raw_test_string, strlen(raw_test_string)) )) {
        NABTO_LOG_ERROR(("Failed read test failed"));
        return false;
    }

    if (buffer_read_uint32(&r_buf, &t32) ||
        buffer_read_uint16(&r_buf, &t16) ||
        buffer_read_uint8(&r_buf, &t8) ||
        buffer_read_raw(&w_buf, &raw_string_buf) ||
        buffer_write_uint32(&w_buf, 0x12345678) ||
        buffer_write_uint16(&w_buf, 0x1234) ||
        buffer_write_uint8(&w_buf, 0x12) || 
        buffer_write_raw(&w_buf, &raw_string_buf) ) {
        NABTO_LOG_ERROR(("Some function should have returned false but returned true"));
        return false;
    }

    return true;

}
