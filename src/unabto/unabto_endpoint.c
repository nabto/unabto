#include "unabto_endpoint.h"

#include "unabto_util.h"

void nabto_ip_convert_v4_mapped_to_v4(const struct nabto_ip_address* a, struct nabto_ip_address* out)
{
    out->type = NABTO_IP_V4;
    READ_U32(out->addr.ipv4, a->addr.ipv6+12);
}

void nabto_ip_convert_v4_to_v4_mapped(const struct nabto_ip_address* a, struct nabto_ip_address* out)
{
    uint8_t v4MappedPrefix[12] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF};
    out->type = NABTO_IP_V6;
    memcpy(out->addr.ipv6, v4MappedPrefix, 12);
    WRITE_U32(out->addr.ipv6+12, a->addr.ipv4);
}

bool nabto_ip_is_v4_mapped(const struct nabto_ip_address* a)
{
    uint8_t v4MappedPrefix[12] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF};
    
    if (a->type == NABTO_IP_V6 && memcmp(a->addr.ipv6, v4MappedPrefix, 12) == 0) {
        return true;
    } else {
        return false;
    }
}

bool nabto_ip_is_equal(const struct nabto_ip_address* a1, const struct nabto_ip_address* a2)
{
    struct nabto_ip_address c1;
    struct nabto_ip_address c2;
    if (nabto_ip_is_v4_mapped(a1)) {
        nabto_ip_convert_v4_mapped_to_v4(a1, &c1);
    } else {
        c1 = *a1;
    }

    if (nabto_ip_is_v4_mapped(a2)) {
        nabto_ip_convert_v4_mapped_to_v4(a2, &c2);
    } else {
        c2 = *a2;
    }
    
    if (c1.type == c2.type) {
        if (c1.type == NABTO_IP_V4) {
            return (c1.addr.ipv4 == c2.addr.ipv4);
        } else if (c1.type == NABTO_IP_V6) {
            return (memcmp(c1.addr.ipv6, c2.addr.ipv6, 16) == 0);
        } else {
            return true;
        }
    }
    return false;
}

bool nabto_ep_is_equal(const nabto_endpoint* ep1, const nabto_endpoint* ep2)
{
    return (ep1->port == ep2->port && nabto_ip_is_equal(&ep1->addr, &ep2->addr));
}


const char* nabto_ip_to_string(const struct nabto_ip_address* addr)
{
    static char output[40]; //0000:1111:2222:3333:4444:5555:6666:7777:8888\0"
    memset(output, 0, 40);
    if (addr->type == NABTO_IP_NONE) {
        sprintf(output, "NABTO_IP_NONE");
    } else if (addr->type == NABTO_IP_ANY) {
        sprintf(output, "NABTO_IP_ANY");
    } else if (addr->type == NABTO_IP_V4) {
        uint32_t ip = addr->addr.ipv4;
        sprintf(output, "%" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8, (uint8_t)(ip >> 24), (uint8_t)(ip >> 16), (uint8_t)(ip >> 8), (uint8_t)(ip));
    } else {
        const uint8_t* ip = addr->addr.ipv6;
        sprintf(output, "%02X%02X:" "%02X%02X:" "%02X%02X:" "%02X%02X:" "%02X%02X:" "%02X%02X:" "%02X%02X:" "%02X%02X", ip[0], ip[1], ip[2], ip[3], ip[4], ip[5], ip[6], ip[7], ip[8], ip[9], ip[10], ip[11], ip[12], ip[13], ip[14], ip[15]);
    }
    return output;
}
