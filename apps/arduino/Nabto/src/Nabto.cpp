#include "Nabto.h"

#define MAX_SOCK_NUM 4

#if NABTO_ENABLE_LOGGING
void p(char *fmt, ... )
{
    static char tmp[128]; // resulting string limited to 128 chars.
    va_list args;
    va_start (args, fmt );
    vsnprintf(tmp, 128, fmt, args);
    va_end (args);
    Serial.print(tmp);
    delay(1);
}
#endif

void NabtoClass::begin(uint8_t* mac, char* name)
{
    // Initialize Ethernet
    spi_initialize();
    network_initialize(mac);

    // Initialize Nabto
    nabto_main_setup* nms = unabto_init_context();
    nms->id = name;

    network_get_current_ip_address(ip);

    uint8_t ip_buffer[4];
    WRITE_U32(ip_buffer, *ip);
    memcpy(&nms->ipAddress, ip_buffer, 4);
    nms->ipAddress = htonl(nms->ipAddress);
    unabto_init();
}

void NabtoClass::version(char* v)
{
  sprintf(v, PRIversion, MAKE_VERSION_PRINTABLE());
}

void NabtoClass::tick()
{
    network_tick();
    unabto_tick();
}

/**
 * Create a default implementation of nabto.
 */

NabtoClass Nabto;

/**
 * Implement network related functions.
 */

void nabto_random(uint8_t* buf, size_t len)
{
    size_t ix;
    for (ix = 0; ix < len; ++ix) {
        *buf++ = (uint8_t)random(0,255);
    }
}

bool nabto_init_socket(uint32_t localAddr, uint16_t* localPort, nabto_socket_t* sd)
{
    return w5100_nabto_init_socket(localAddr, localPort, sd);
}

void nabto_close_socket(nabto_socket_t* socketDescriptor)
{
    w5100_nabto_close_socket(socketDescriptor);
}

ssize_t nabto_read(nabto_socket_t socket,
                   uint8_t*       buf,
                   size_t         len,
                   uint32_t*      addr,
                   uint16_t*      port)
{
    return w5100_nabto_read(socket, buf, len, addr, port);
}

ssize_t nabto_write(nabto_socket_t socket,
                    const uint8_t* buf,
                    size_t         len,
                    uint32_t       addr,
                    uint16_t       port)
{
    return w5100_nabto_write(socket, buf, len, addr, port);
}

/**
 * Platform related functions.
 */

bool nabto_init_platform()
{
    return true;
}

void nabto_close_platform()
{
}

/**
 * Time related implementations.
 */

#define MAX_STAMP_DIFF 0x7fffffff;

bool nabtoIsStampPassed(nabto_stamp_t *stamp)
{
    return *stamp - nabtoGetStamp() > (uint32_t)MAX_STAMP_DIFF;
}

nabto_stamp_diff_t nabtoStampDiff(nabto_stamp_t * newest, nabto_stamp_t * oldest)
{
    return (*newest - *oldest);
}

int nabtoStampDiff2ms(nabto_stamp_diff_t diff)
{
    return (int) diff;
}

#ifdef __cplusplus
extern "C" {
#endif

void w5100_set_chip_select_pin(bool state)
{
    if(state)
    {
        digitalWrite(10, HIGH);
    }
    else
    {
        digitalWrite(10, LOW);
    }
}

#ifdef __cplusplus
}
#endif
