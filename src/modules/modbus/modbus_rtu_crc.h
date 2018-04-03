/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _MODBUS_RTU_CRC_H_
#define _MODBUS_RTU_CRC_H_

#include <unabto/unabto_env_base.h>

#ifdef __cplusplus
extern "C" {
#endif

bool modbus_rtu_crc_verify_crc_field(uint8_t* buffer, uint16_t length);
void modbus_rtu_crc_update_crc_field(uint8_t* buffer, uint16_t length);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
