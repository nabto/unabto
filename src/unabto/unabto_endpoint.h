/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_ENDPOINT_H_
#define _UNABTO_ENDPOINT_H_

#if NABTO_SLIM
#include <unabto_platform_types.h>
#else
#include <unabto/unabto_env_base.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif

enum nabto_ip_address_type {
    NABTO_IP_NONE = 0,
    NABTO_IP_ANY,
    NABTO_IP_V4,
    NABTO_IP_V6
};

struct nabto_ip_address {
    enum nabto_ip_address_type type;

    union {
        uint32_t ipv4; ///< IP address
        uint8_t  ipv6[16];
    } addr;
     
};

/** 
 * The IP endpoint type.
 */
typedef struct {
    uint16_t port; ///< port number
    struct nabto_ip_address addr;
        
} nabto_endpoint;



void nabto_ip_convert_v4_mapped_to_v4(const struct nabto_ip_address* a, struct nabto_ip_address* out);

void nabto_ip_convert_v4_to_v4_mapped(const struct nabto_ip_address* a, struct nabto_ip_address* out);

bool nabto_ip_is_v4_mapped(const struct nabto_ip_address* a);

bool nabto_ip_is_equal(const struct nabto_ip_address* a1, const struct nabto_ip_address* a2);

bool nabto_ep_is_equal(const nabto_endpoint* ep1, const nabto_endpoint* ep2);

const char* nabto_ip_to_string(const struct nabto_ip_address* addr);


#ifdef __cplusplus
} //extern "C"
#endif

#endif // UNABTO_ENDPOINT_H
