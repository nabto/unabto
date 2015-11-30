/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_NETWORK

#include "tcp_provider.h"
#include <unabto/unabto_env_base.h>
#include <unabto/unabto_util.h>
#include <uart.h>

#define UART_CHANNEL                                0

void tcp_provider_initialize(void)
{ }

void tcp_provider_tick(void)
{ }

void tcp_provider_shutdown(void)
{ }

bool tcp_provider_connect(tcp_socket* tcpSocket, text host, uint16_t port)
{
    char connectionString[100];
    uint16_t connectionTimer = 2000;

    *tcpSocket = 0;

    sprintf(connectionString, "C%" PRItext "/%" PRIu16 "\n", host, port);

    uart_flush_receiver(UART_CHANNEL);
    
    if(uart_can_write(UART_CHANNEL) < strlen(connectionString))
    {
        NABTO_LOG_ERROR(("Unable to send connection string to MatchPort!"));
        return false;
    }

    uart_write_buffer(UART_CHANNEL, connectionString, strlen(connectionString));
    while(uart_can_read(UART_CHANNEL) == 0 && connectionTimer-- > 0)
    {
        Sleep(1);
    }

    if(uart_can_read(UART_CHANNEL) == 0 || uart_read(UART_CHANNEL) != 'C')
    {
        NABTO_LOG_ERROR(("Unable to connect to TCP relay service!"));
        return false;
    }

    NABTO_LOG_TRACE(("Connection to TCP relay service established."));

    return true;
}

void tcp_provider_disconnect(tcp_socket* tcpSocket)
{
    NABTO_LOG_TRACE(("TCP connection closed."));
}

uint16_t tcp_provider_read(tcp_socket* tcpSocket, uint8_t* buffer, uint16_t maximumLength)
{
    uint16_t length;

    length = uart_can_read(UART_CHANNEL);

    if(length > maximumLength)
    {
        length = maximumLength;
    }

    if(length > 0)
    {
        uart_read_buffer(UART_CHANNEL, buffer, length);
    }

    return length;
}

void tcp_provider_write(tcp_socket* tcpSocket, uint8_t* buffer, uint16_t length)
{
    if(uart_can_write(UART_CHANNEL) >= length)
    {
        uart_write_buffer(UART_CHANNEL, buffer, length);
    }
}
