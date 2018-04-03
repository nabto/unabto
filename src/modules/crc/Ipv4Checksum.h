/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _IPV4_CHECKSUM_H_
#define _IPV4_CHECKSUM_H_

#include <unabto/unabto_env_base.h>

#ifdef __cplusplus
extern "C" {
#endif

uint16_t Ipv4Checksum_Calculate(uint8_t* buffer, uint16_t offset, uint16_t length);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
