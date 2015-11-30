/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#ifndef _UART_H_
#define _UART_H_

#include <unabto_platform_types.h>

#ifdef __cplusplus
extern "C" {
#endif

void uart_initialize(uint32_t baudrate);
bool uart_overrun(bool* queueOverrun);
uint16_t uart_can_read(void);
uint16_t uart_can_write(void);
uint8_t uart_read(void);
void uart_read_buffer(void* buffer, uint16_t length);
void uart_write(uint8_t value);
void uart_write_buffer(const void* buffer, uint16_t length);
void uart_flush_receiver(void);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
