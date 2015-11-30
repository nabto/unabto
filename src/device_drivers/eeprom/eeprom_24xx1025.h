/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _EEPROM_H_
#define _EEPROM_H_

#ifdef __cplusplus
extern "C" {
#endif

bool eeprom_write(uint32_t address, void* data, uint16_t length);
bool eeprom_read(uint32_t address, void* data, uint16_t length);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
