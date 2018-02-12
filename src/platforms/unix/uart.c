/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_PERIPHERAL

#include "uart.h"
#include "unabto/util/unabto_queue.h"
#include <unabto/unabto_util.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

typedef struct
{
    queue_t receiveQueue;
    uint8_t receiveQueueBuffer[4096];
    int fileDescriptor;
} uart_channel;

static bool low_level_read_from_uart(uart_channel* uartChannel);

static uart_channel channels[UART_NUMBER_OF_CHANNELS];

void uart_initialize(uint8_t channel, void* name, uint32_t baudrate, uint8_t databits, uart_parity parity, uart_stopbits stopbits)
{
    uart_channel* uartChannel;
    const char* _name = (const char*)name;
    struct termios tios;

    if(channel >= UART_NUMBER_OF_CHANNELS)
    {
        NABTO_LOG_FATAL(("Invalid UART channel specified!"));
    }

    uartChannel = &channels[channel];

    queue_init(&uartChannel->receiveQueue, uartChannel->receiveQueueBuffer, sizeof(uartChannel->receiveQueueBuffer));

    uartChannel->fileDescriptor = open(_name, O_RDWR | O_NOCTTY | O_NONBLOCK);

    if(uartChannel->fileDescriptor == -1)
    {
        NABTO_LOG_FATAL(("Unable to open UART '%s'!", _name));
    }

    memset(&tios, 0, sizeof(tios));
    tios.c_iflag = IGNBRK | IGNCR;
    tios.c_iflag |= ((tcflag_t) INPCK);
    tios.c_oflag &= ~OPOST;
    tios.c_lflag &= ~ICANON;
    cfmakeraw(&tios); // :...
    tios.c_cc[VMIN] = 0;
    tios.c_cc[VTIME] = 0;
    tios.c_cflag = CLOCAL | CREAD;

    switch(parity)
    {
        case UART_PARITY_NONE:
            break;
            
        case UART_PARITY_EVEN:
            tios.c_cflag |= ((tcflag_t) PARENB);
            break;
            
        case UART_PARITY_ODD:
            tios.c_cflag |= ((tcflag_t) PARENB | PARODD);
            break;
            
        // case UART_PARITY_MARK:
            // tios.c_cflag |= ((tcflag_t) PARENB | CMSPAR | PARODD);
            // break;
            
        // case UART_PARITY_SPACE:
            // tios.c_cflag |= ((tcflag_t) PARENB | CMSPAR);
            // break;

        default:
            NABTO_LOG_FATAL(("Invalid number of databits for UART!"));
    }

    switch(stopbits)
    {
        case UART_STOPBITS_ONE:
            break;

        case UART_STOPBITS_TWO:
            tios.c_cflag |= ((tcflag_t) CSTOPB);
            break;

        default:
            NABTO_LOG_FATAL(("Invalid number of stopbits for UART!"));
    }

    switch(databits)
    {
        case 5:
            tios.c_cflag |= ((tcflag_t) CS5);
            break;
        case 6:
            tios.c_cflag |= ((tcflag_t) CS6);
            break;
        case 7:
            tios.c_cflag |= ((tcflag_t) CS7);
            break;
        case 8:
            tios.c_cflag |= ((tcflag_t) CS8);
            break;
        default:
            NABTO_LOG_FATAL(("Invalid number of databits specified for UART '%s'.", _name));
            return;
    }

    switch(baudrate)
    {
        case 300:
            cfsetispeed(&tios, B300);
            cfsetospeed(&tios, B300);
        case 600:
            cfsetispeed(&tios, B600);
            cfsetospeed(&tios, B600);
        case 1200:
            cfsetispeed(&tios, B1200);
            cfsetospeed(&tios, B1200);
        case 2400:
            cfsetispeed(&tios, B2400);
            cfsetospeed(&tios, B2400);
            break;
        case 4800:
            cfsetispeed(&tios, B4800);
            cfsetospeed(&tios, B4800);
            break;
        case 9600:
            cfsetispeed(&tios, B9600);
            cfsetospeed(&tios, B9600);
            break;
        case 19200:
            cfsetispeed(&tios, B19200);
            cfsetospeed(&tios, B19200);
            break;
        case 38400:
            cfsetispeed(&tios, B38400);
            cfsetospeed(&tios, B38400);
            break;
        case 57600:
            cfsetispeed(&tios, B57600);
            cfsetospeed(&tios, B57600);
            break;
        case 115200:
            cfsetispeed(&tios, B115200);
            cfsetospeed(&tios, B115200);
            break;
        default:
            NABTO_LOG_FATAL(("Invalid baudrate specified for UART '%s'.", _name));
            return;
    }

    if (-1 == tcsetattr(uartChannel->fileDescriptor, TCSANOW, &tios))
    {
        NABTO_LOG_FATAL(("Unable to configure UART '%s'.", _name));
    }

    NABTO_LOG_TRACE(("UART '%s' opened with handle %i.", _name, uartChannel->fileDescriptor));
}

void uart_shutdown(uint8_t channel)
{
    uart_channel* uartChannel;
    
    if(channel >= UART_NUMBER_OF_CHANNELS)
    {
        NABTO_LOG_FATAL(("Invalid UART channel specified!"));
    }

    uartChannel = &channels[channel];

    if(uartChannel->fileDescriptor > -1)
    {
        close(uartChannel->fileDescriptor);
        uartChannel->fileDescriptor = -1;
    }
}

uint16_t uart_can_read(uint8_t channel)
{
    uart_channel* uartChannel;
    
    if(channel >= UART_NUMBER_OF_CHANNELS)
    {
        NABTO_LOG_FATAL(("Invalid UART channel specified!"));
        return 0;
    }

    uartChannel = &channels[channel];

    low_level_read_from_uart(uartChannel); // ensure that all data has been read into the driver buffer
    
    return queue_count(&uartChannel->receiveQueue);
}

uint16_t uart_can_write(uint8_t channel)
{
    if(channel >= UART_NUMBER_OF_CHANNELS)
    {
        NABTO_LOG_FATAL(("Invalid UART channel specified!"));
        return 0;
    }

    return 4096;
}

uint8_t uart_read(uint8_t channel)
{
    uint8_t value;
    uint16_t count;
    uart_channel* uartChannel;
    struct timespec sleepTime;
    
    if(channel >= UART_NUMBER_OF_CHANNELS)
    {
        NABTO_LOG_FATAL(("Invalid UART channel specified!"));
        return 0;
    }

    uartChannel = &channels[channel];

    sleepTime.tv_sec = 0;
    sleepTime.tv_nsec = 1000000;

    while(queue_is_empty(&uartChannel->receiveQueue))
    {
        if(low_level_read_from_uart(uartChannel) == false)
        {
            nanosleep(&sleepTime, NULL);
        }
    }

    queue_dequeue(&uartChannel->receiveQueue, &value);

    count = uart_can_read(channel);

    return value;
}

void uart_read_buffer(uint8_t channel, void* buffer, uint16_t length)
{
    uint8_t* b = (uint8_t*) buffer;
    uint16_t originalLength = length;
    
    if(channel >= UART_NUMBER_OF_CHANNELS)
    {
        NABTO_LOG_FATAL(("Invalid UART channel specified!"));
        return;
    }

    while (length--)
    {
        *b++ = uart_read(channel);
    }

    NABTO_LOG_TRACE(("Read %i bytes from UART.", (int)originalLength));
}

void uart_write(uint8_t channel, uint8_t value)
{
    if(channel >= UART_NUMBER_OF_CHANNELS)
    {
        NABTO_LOG_FATAL(("Invalid UART channel specified!"));
        return;
    }

    uart_write_buffer(channel, &value, 1);
}

void uart_write_buffer(uint8_t channel, const void* buffer, uint16_t length)
{
    ssize_t bytesWritten;
    uart_channel* uartChannel;
    
    if(channel >= UART_NUMBER_OF_CHANNELS)
    {
        NABTO_LOG_FATAL(("Invalid UART channel specified!"));
        return;
    }

    uartChannel = &channels[channel];

    bytesWritten = write(uartChannel->fileDescriptor, buffer, (size_t)length);

    if(bytesWritten < 0)
    {
        NABTO_LOG_FATAL(("Error writing bytes to UART with handle %i (error code=%i).", uartChannel->fileDescriptor, (int)bytesWritten));
    }

    if(length != bytesWritten)
    {
        NABTO_LOG_FATAL(("Unable to write all bytes to the UART (only %" PRIu16 " out of %" PRIu16 " was written)!", bytesWritten, length));
    }

    NABTO_LOG_TRACE(("Wrote %i bytes to UART.", (int)length));
}

void uart_flush_receiver(uint8_t channel)
{
    uart_channel* uartChannel;
    
    if(channel >= UART_NUMBER_OF_CHANNELS)
    {
        NABTO_LOG_FATAL(("Invalid UART channel specified!"));
        return;
    }

    uartChannel = &channels[channel];

    while(low_level_read_from_uart(uartChannel)); // keep reading until no more data is available

    queue_reset(&uartChannel->receiveQueue);
}

void uart_flush_transmitter(uint8_t channel)
{   
    uart_channel* uartChannel;
    
    if(channel >= UART_NUMBER_OF_CHANNELS)
    {
        NABTO_LOG_FATAL(("Invalid UART channel specified!"));
        return;
    }

    uartChannel = &channels[channel];

    tcdrain(uartChannel->fileDescriptor);
}

static bool low_level_read_from_uart(uart_channel* uartChannel)
{
    uint8_t buffer[1024];
    ssize_t bytesRead;
    uint16_t i;
    
    if(queue_free(&uartChannel->receiveQueue) == 0)
    {
        return false;
    }

    bytesRead = read(uartChannel->fileDescriptor, buffer, MIN(sizeof(buffer), queue_free(&uartChannel->receiveQueue)));
    if(bytesRead < 0)
    {
        NABTO_LOG_ERROR(("Unable to read from UART!"));
        return false;
    }

    if(bytesRead > 0)
    {
        uint16_t length = MIN(queue_free(&uartChannel->receiveQueue), bytesRead);
        queue_enqueue_array(&uartChannel->receiveQueue, buffer, length);

        if(length != bytesRead)
        {
            NABTO_LOG_TRACE(("UART buffer overflow!"));
        }
        // for(i = 0; i < bytesRead; i++)
        // {
            // queue_enqueue(&uartChannel->receiveQueue, buffer[i]);
        // }
        return true;
    }
    else
    {
        return false;
    }
}
