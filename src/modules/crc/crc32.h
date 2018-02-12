/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _CRC32_H_
#define _CRC32_H_

#include <unabto/unabto_env_base.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t crc32_calculate(const void* data, uint16_t length);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
