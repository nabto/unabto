/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#include "uart.h"

static far rom const uint8_t letters[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

void uart_initialize(uint32_t baudrate) {
    TXSTA = 0x20;
    RCSTA = 0x90;

    if (((GetPeripheralClock() + 2ul * baudrate) / baudrate / 4ul - 1ul) <= 255ul) // See if we can use the high baud rate setting
    {
        SPBRG = (GetPeripheralClock() + 2ul * baudrate) / baudrate / 4ul - 1ul;
        TXSTAbits.BRGH = 1;
    } else // Use the low baud rate setting
    {
        SPBRG = (GetPeripheralClock() + 8ul * baudrate) / baudrate / 16ul - 1ul;
    }
}

bool uart_overrun(void) {
    return RCSTAbits.OERR;
}

bool uart_framing_error(void) {
    return RCSTAbits.FERR;
}

bool uart_can_read(void) {
    return PIR1bits.RCIF;
}

bool uart_can_write(void) {
    return PIR1bits.TXIF == 1;
}

uint8_t uart_read(void) {
    uint8_t value;

    while (uart_can_read() == false);

    value = RCREG;

    if (RCSTAbits.OERR) {
        RCSTAbits.CREN = 0;
        RCSTAbits.CREN = 1;
    }

    return value;
}

void uart_write(uint8_t value) {
    while (uart_can_write() == false);

    TXREG = value;
}

void uart_write_string_rom(const far rom char* string) {
    while (*string) {
        uart_write(*string++);
    }
}

void uart_write_string(const char* string) {
    while (*string) {
        uart_write(*string++);
    }
}

void uart_write_buffer(const uint8_t* buffer, uint16_t length) {
    while (length--) {
        //debug_channel_write(*buffer);
        uart_write(*buffer++);
    }
}

void uart_write_uint16(uint16_t value) {
    char buffer[6];

    uitoa(value, (BYTE*) buffer);

    uart_write_string(buffer);
}

void uart_write_uint8_hex(uint8_t value) {
    uart_write(letters[value >> 4]);
    uart_write(letters[value & 0x0f]);
}

void uart_write_ip(const uint8_t* ip) {
    uint8_t i;

    for (i = 0; i < 4; i++) {
        uart_write_uint16(ip[i]);

        if (i < (4 - 1)) {
            uart_write('.');
        }
    }
}

#if NABTO_USE_DEBUG_CHANNEL != 1

int _user_putc(char c) {
    uart_write(c);
    return c;
}

#endif
