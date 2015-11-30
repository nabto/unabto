/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#ifndef _FLASH_IO_DRIVER_H_
#define _FLASH_IO_DRIVER_H_

#include <unabto/unabto_env_base.h>

#ifdef __cplusplus
extern "C" {
#endif

// Erase a 1024 byte block of flash.
bool flash_io_driver_erase_block(__ROM void* address);

// Write a 64 byte block to flash.
// The destination flash MUST be erased before writing!
bool flash_io_driver_write_block(__ROM void* address, const void* data);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
