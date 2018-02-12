/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UART_H_
#define _UART_H_

#include <unabto/unabto_env_base.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    UART_PARITY_NONE,
    UART_PARITY_EVEN,
    UART_PARITY_ODD,
    UART_PARITY_MARK,
    UART_PARITY_SPACE
} uart_parity;

typedef enum
{
    UART_STOPBITS_ONE,
    UART_STOPBITS_TWO
} uart_stopbits;

void uart_initialize(uint8_t channel, const void* name, uint32_t baudrate, uint8_t databits, uart_parity parity, uart_stopbits stopbits);
void uart_shutdown(uint8_t channel);

// Returns the number of bytes that can be read from the UART without blocking.
uint16_t uart_can_read(uint8_t channel);

// Returns the number of bytes that can be written to the UART without blocking.
uint16_t uart_can_write(uint8_t channel);

// Read a single byte from the UART - block if needed.
uint8_t uart_read(uint8_t channel);

// Read the specified number of bytes from the UART - block if needed.
void uart_read_buffer(uint8_t channel, void* buffer, uint16_t length);

// Write a single byte to the UART - block if needed.
void uart_write(uint8_t channel, uint8_t value);

// Write the specified number of bytes to the UART - block if needed.
void uart_write_buffer(uint8_t channel, const void* buffer, uint16_t length);

// Dump all bytes from the receive buffer.
void uart_flush_receiver(uint8_t channel);

// Block until the transmission buffer is empty.
void uart_flush_transmitter(uint8_t channel);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
