/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#ifndef _IIC_H_
#define _IIC_H_

#include <unabto_platform_types.h>

#ifdef __cplusplus
extern "C" {
#endif

void iic_initialize(uint8_t channel);
void iic_start(uint8_t channel);
void iic_repeated_start(uint8_t channel);
void iic_stop(uint8_t channel);
void iic_read(uint8_t channel, void* data, uint16_t length);
bool iic_write(uint8_t channel, void* data, uint16_t length);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
