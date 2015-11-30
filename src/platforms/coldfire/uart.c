/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#include <unabto_env_base.h>
#include <devices/coldfire/uart.h>
#include "mcf52255.h"

static void _uitoa(uint16_t value, uint8_t* buffer);

static const uint8_t letters[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

void uart_initialize(uint8_t channel, uint32_t baudrate)
{
    uint16_t ubgs;
    
    MCF_UART_UCR( channel) = MCF_UART_UCR_RESET_TX; // Reset Transmitter
    MCF_UART_UCR( channel) = MCF_UART_UCR_RESET_RX; // Reset Receiver
    MCF_UART_UCR( channel) = MCF_UART_UCR_RESET_MR; // Reset Mode Register
    MCF_UART_UMR( channel) = (MCF_UART_UMR_PM_NONE | MCF_UART_UMR_BC_8); // No parity, 8-bits per character
    MCF_UART_UMR( channel) = (MCF_UART_UMR_CM_NORMAL | MCF_UART_UMR_SB_STOP_BITS_1); // No echo or loop back, 1 stop bit
    MCF_UART_UCSR( channel) = (MCF_UART_UCSR_RCS_SYS_CLK | MCF_UART_UCSR_TCS_SYS_CLK); // Set Rx and Tx baud by SYSTEM CLOCK
    MCF_UART_UIMR( channel) = 0; // Mask all UART interrupts

    ubgs = (uint16)((SYSTEM_CLOCK_KHZ * 1000) / (baudrate * 32)); // Calculate baud settings

    MCF_UART_UBG1( channel) = (uint8)((ubgs & 0xff00) >> 8);
    MCF_UART_UBG2( channel) = (uint8)(ubgs & 0x00ff);
    
    MCF_UART_UCR( channel) = MCF_UART_UCR_TX_ENABLED | MCF_UART_UCR_RX_ENABLED; // Enable receiver and transmitter
}

//bool uart_overrun(void) {
//    return RCSTAbits.OERR;
//}

//bool uart_framing_error(void) {
//    return RCSTAbits.FERR;
//}

bool uart_data_ready(uint8_t channel)
{
    return (MCF_UART_USR(channel) & MCF_UART_USR_RXRDY);
}

uint8_t uart_read(uint8_t channel)
{
    // Wait until character has been received
    while (!uart_data_ready(channel))
        ;
    
    return MCF_UART_URB(channel);
}

void uart_write(uint8_t channel, uint8_t value)
{
    // Wait until space is available in the FIFO
    while ((MCF_UART_USR(channel) & MCF_UART_USR_TXRDY) == false)
        ;
    
    // Send the character
    MCF_UART_UTB( channel) = value;
}

void uart_write_string(uint8_t channel, const char* string)
{
    while (*string){
        uart_write(channel, *string++);
    }
}

void uart_write_uint16(uint8_t channel, uint16_t value)
{
    char buffer[6];
    
    _uitoa(value, (uint8_t*)buffer);
    
    uart_write_string(channel, (const char*)buffer);
}

void uart_write_uint8_hex(uint8_t channel, uint8_t value)
{
    uart_write(channel, letters[value >> 4]);
    uart_write(channel, letters[value & 0x0f]);
}

void uart_write_ip(uint8_t channel, const uint8_t* ip)
{
    uint8_t i;
    
    for (i = 0; i < 4; i++){
        uart_write_uint16(channel, ip[i]);
        
        if (i < (4 - 1)){
            uart_write(channel, '.');
        }
    }
}

static void _uitoa(uint16_t value, uint8_t* buffer)
{
    uint8_t i;
    uint16_t digit;
    uint16_t divisor;
    bool printed = false;
    
    if (value){
        for (i = 0, divisor = 10000; i < 5u; i++){
            digit = value / divisor;
            if (digit || printed){
                *buffer++ = (uint8_t)('0' + digit);
                value -= digit * divisor;
                printed = true;
            }
            divisor /= 10;
        }
    }else{
        *buffer++ = '0';
    }
    
    *buffer = '\0';
}
