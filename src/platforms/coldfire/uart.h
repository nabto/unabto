/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UART_H_
#define _UART_H_

#include <devices/coldfire/unabto_env_base_coldfire.h>

#ifdef __cplusplus
extern "C" {
#endif

//#if NABTO_BUILDING_BOOTLOADER == 1
//#include "platform/platform.h"
//#else
//#include "unabto_env_base.h"
//#endif

// required due to code warrior console stuff that must be removed from project
//#include "C:/Freescale/CW MCU v10.2/MCU/ColdFire_Support/ewl/EWL_C/include/sys/UART.h"

// make CodeWarrior happy
//#define uart_init(channel, systemClockKHz, baudRate) uart_initialize(baudRate)
//#define uart_getchar(channel) uart_read()
//#define uart_putchar(channel, ch) uart_write(ch)
//#define InitializeUART(baudRate) uart_initialize(baudRate)

void uart_initialize(uint8_t channel, uint32_t baudrate);
bool uart_overrun(uint8_t channel);
bool uart_framing_error(uint8_t channel);
bool uart_data_ready(uint8_t channel);
uint8_t uart_read(uint8_t channel);
void uart_write(uint8_t channel, uint8_t value);
#define uart_write_string_rom(channel, string) uart_write_string(channel, string)
void uart_write_string(uint8_t channel, const char* string);
void uart_write_uint16(uint8_t channel, uint16_t value);
void uart_write_uint8_hex(uint8_t channel, uint8_t value);
void uart_write_ip(uint8_t channel, const uint8_t* ip);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
