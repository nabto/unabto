/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UART_H_
#define _UART_H_

#if NABTO_BUILDING_BOOTLOADER == 1
#include "platform/platform.h"
#else
#include "unabto_env_base.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

void uart_initialize(uint32_t baudrate);
bool uart_overrun(void);
bool uart_framing_error(void);
//bool uart_data_ready(void);
bool uart_can_read(void);
bool uart_can_write(void);
uint8_t uart_read(void);
void uart_write(uint8_t value);
void uart_write_string_rom(const far rom char* string);
void uart_write_string(const char* string);
void uart_write_buffer(const uint8_t* buffer, uint16_t length);
void uart_write_uint16(uint16_t value);
void uart_write_uint8_hex(uint8_t value);
void uart_write_ip(const uint8_t* ip);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
