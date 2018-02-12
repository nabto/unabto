/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _EEPROM_H_
#define _EEPROM_H_

#include <unabto_platform_types.h>

#ifdef __cplusplus
extern "C" {
#endif

bool eeprom_write(uint8_t channel, uint32_t address, const void* data, uint16_t length);
bool eeprom_read(uint8_t channel, uint32_t address, void* data, uint16_t length);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
