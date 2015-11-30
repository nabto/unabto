/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#ifndef _FLASH_IO_DRIVER_H_
#define _FLASH_IO_DRIVER_H_

#include <unabto/unabto_env_base.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FLASH_ERASE_BLOCK_SIZE           (512)
#define FLASH_WRITE_BLOCK_SIZE           (4)

bool flash_io_driver_initialize(void);
  
// Erase a 512 byte block of flash.
bool flash_io_driver_erase_block(__ROM void* address);

// Write to flash.
// The destination flash MUST be erased before writing!
bool flash_io_driver_write(__ROM void* address, const void* data, uint16_t length);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
