/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_PERIPHERAL

#include "uart.h"
#include <unabto/util/unabto_queue.h>
#include <unabto/unabto_util.h>

#define RECEIVE_QUEUE_BUFFER_SIZE     4096
#define TRANSMIT_QUEUE_BUFFER_SIZE    4096

typedef struct
{
  queue_t receiveQueue;
  uint8_t receiveQueueBuffer[RECEIVE_QUEUE_BUFFER_SIZE];
  HANDLE hPort;
  DCB dcb;
  COMMTIMEOUTS timeouts;
} uart_channel;

static bool low_level_read_from_uart(uart_channel* uartChannel);
static uart_channel* look_up_uart_channel(uint8_t channel);

static uart_channel channels[UART_NUMBER_OF_CHANNELS];

void uart_initialize(uint8_t channel, const void* name, uint32_t baudrate, uint8_t databits, uart_parity parity, uart_stopbits stopbits)
{
  const char* pipeIdentifier = "\\\\.\\pipe\\";
  int pipeIdentifierLength = strlen(pipeIdentifier);
  char nameBuffer[1024];

  uart_channel* uartChannel = look_up_uart_channel(channel);

  queue_init(&uartChannel->receiveQueue, uartChannel->receiveQueueBuffer, sizeof(uartChannel->receiveQueueBuffer));

  if (memcmp(pipeIdentifier, name, pipeIdentifierLength) != 0 && strlen(name) > 4 && ((const char*)name)[0] != '\\') // it's a com port higher than 9
  {
    sprintf_s(nameBuffer, sizeof(nameBuffer), "\\\\.\\%s", name);
    name = nameBuffer;
  }

  uartChannel->hPort = CreateFileA((LPCSTR)name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
  if (uartChannel->hPort == INVALID_HANDLE_VALUE)
  {
    //DWORD error = GetLastError();
    //char* errorString = strerror(error);
    //NABTO_LOG_TRACE(("Error:%l", error));
    NABTO_LOG_FATAL(("Unable to open COM port '%s'!", (const char*)name));
  }

  if (memcmp(pipeIdentifier, name, pipeIdentifierLength) == 0) // it's a pipe
  {
    DWORD mode = PIPE_READMODE_BYTE | PIPE_NOWAIT;
    if (!SetNamedPipeHandleState(uartChannel->hPort, &mode, NULL, NULL))
    {
      //DWORD error = GetLastError();
      //char* errorString = strerror(error);
      //NABTO_LOG_TRACE(("Error:%l", error));
      NABTO_LOG_FATAL(("Unable to make pipe non-blocking '%s'!", (const char*)name));
    }
  }
  else // it's a COM port
  {
    // Configure com port
    memset(&uartChannel->dcb, 0, sizeof(DCB));
    uartChannel->dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(uartChannel->hPort, &uartChannel->dcb))
    {
      NABTO_LOG_FATAL(("Unable to get COM port state!"));
    }

    uartChannel->dcb.BaudRate = baudrate;

    if (databits < 5 || 8 < databits)
    {
      NABTO_LOG_FATAL(("Invalid number of databits for UART!"));
    }
    uartChannel->dcb.ByteSize = databits;

    switch (parity)
    {
      case UART_PARITY_NONE:
        uartChannel->dcb.Parity = NOPARITY;
        break;

      case UART_PARITY_EVEN:
        uartChannel->dcb.fParity = TRUE;
        uartChannel->dcb.fErrorChar = FALSE;
        uartChannel->dcb.Parity = EVENPARITY;
        break;

      case UART_PARITY_ODD:
        uartChannel->dcb.fParity = TRUE;
        uartChannel->dcb.fErrorChar = FALSE;
        uartChannel->dcb.Parity = ODDPARITY;
        break;

      case UART_PARITY_MARK:
        uartChannel->dcb.fParity = TRUE;
        uartChannel->dcb.fErrorChar = FALSE;
        uartChannel->dcb.Parity = MARKPARITY;
        break;

      case UART_PARITY_SPACE:
        uartChannel->dcb.fParity = TRUE;
        uartChannel->dcb.fErrorChar = FALSE;
        uartChannel->dcb.Parity = SPACEPARITY;
        break;

      default:
        NABTO_LOG_FATAL(("Invalid number of databits for UART!"));
    }

    switch (stopbits)
    {
      case UART_STOPBITS_ONE:
        uartChannel->dcb.StopBits = ONESTOPBIT;
        break;

      case UART_STOPBITS_TWO:
        uartChannel->dcb.StopBits = TWOSTOPBITS;
        break;

      default:
        NABTO_LOG_FATAL(("Invalid number of stopbits for UART!"));
    }

    uartChannel->dcb.fBinary = TRUE;

    if (!SetCommState(uartChannel->hPort, &uartChannel->dcb))
    {
      NABTO_LOG_FATAL(("Unable to set COM port state!"));
    }

    // set event masks so the receive queue can be polled.
    if (!SetCommMask(uartChannel->hPort, EV_RXCHAR | EV_ERR))
    {
      DWORD error = GetLastError();
      NABTO_LOG_FATAL(("Unable to set COM port state (%lu)!", error));
    }

    // set read timeouts so ReadFile performs non-blocking reads.
    // RMW the port's timeouts structure
    if (!GetCommTimeouts(uartChannel->hPort, &uartChannel->timeouts))
    {
      NABTO_LOG_FATAL(("Unable to get COM port timeouts!"));
    }

    uartChannel->timeouts.ReadIntervalTimeout = MAXDWORD;
    uartChannel->timeouts.ReadTotalTimeoutConstant = 1;
    uartChannel->timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
    uartChannel->timeouts.WriteTotalTimeoutConstant = 200;
    uartChannel->timeouts.WriteTotalTimeoutMultiplier = 1;

    if (!SetCommTimeouts(uartChannel->hPort, &uartChannel->timeouts))
    {
      NABTO_LOG_FATAL(("Unable to set COM port timeouts!"));
    }
  }
}

void uart_shutdown(uint8_t channel)
{
  uart_channel* uartChannel = look_up_uart_channel(channel);

  if (uartChannel->hPort != INVALID_HANDLE_VALUE)
  {
    CloseHandle(uartChannel->hPort);
    uartChannel->hPort = INVALID_HANDLE_VALUE;
  }
}

uint16_t uart_can_read(uint8_t channel)
{
  uart_channel* uartChannel = look_up_uart_channel(channel);

  while (low_level_read_from_uart(uartChannel));

  return queue_count(&uartChannel->receiveQueue);
}

uint16_t uart_can_write(uint8_t channel)
{
  look_up_uart_channel(channel); // just used to validate channel number

  return TRANSMIT_QUEUE_BUFFER_SIZE; // assuming we can always write a lot of data to it due to OS buffering
}

uint8_t uart_read(uint8_t channel)
{
  uint8_t value;
  uart_channel* uartChannel = look_up_uart_channel(channel);

  // block until data is received
  while (queue_is_empty(&uartChannel->receiveQueue))
  {
    if (low_level_read_from_uart(uartChannel) == false)
    {
      Sleep(1);
    }
  }

  queue_dequeue(&uartChannel->receiveQueue, &value);

  return value;
}

void uart_read_buffer(uint8_t channel, void* buffer, uint16_t length)
{
  uint8_t* b = (uint8_t*)buffer;
  uint16_t originalLength = length;

  while (length--)
  {
    *b++ = uart_read(channel);
  }

  NABTO_LOG_TRACE(("Read %u bytes from UART.", (int)originalLength));
}

void uart_write(uint8_t channel, uint8_t value)
{
  uart_write_buffer(channel, &value, 1);
}

void uart_write_buffer(uint8_t channel, const void* buffer, uint16_t length)
{
  DWORD bytesWritten;
  uart_channel* uartChannel = look_up_uart_channel(channel);

  WriteFile(uartChannel->hPort, buffer, length, &bytesWritten, NULL);

  if (length != bytesWritten)
  {
    NABTO_LOG_WARN(("Unable to write all bytes to the UART (only %u out of %u was written)!", (int)bytesWritten, (int)length));
  }

  NABTO_LOG_TRACE(("Wrote %u bytes to UART.", (int)bytesWritten));
}

void uart_flush_receiver(uint8_t channel)
{
  uart_channel* uartChannel = look_up_uart_channel(channel);

  while (low_level_read_from_uart(uartChannel));

  queue_reset(&uartChannel->receiveQueue);
}

void uart_flush_transmitter(uint8_t channel)
{
  look_up_uart_channel(channel); // just used to validate channel number

  // can't do anything here
}

// move as much data as possible from the UART OS buffer to the UART driver receive buffer.
static bool low_level_read_from_uart(uart_channel* uartChannel)
{
  uint8_t buffer[1024];
  DWORD count;
  uint16_t length;

  count = queue_free(&uartChannel->receiveQueue);

  if (count == 0) // room for more bytes in the driver receive queue?
  {
    return false; // no
  }

  if (count > sizeof(buffer)) // limit count
  {
    count = sizeof(buffer);
  }

  // try to read as many bytes as there is room for in the driver receive queue
  if (ReadFile(uartChannel->hPort, buffer, count, &count, 0) == false)
  {
    return false; // read failed for some reason
  }

  // no bytes received?
  if (count == 0)
  {
    return false; // no bytes received
  }

  // no bytes received?
  if (count > 0xffff)
  {
    NABTO_LOG_FATAL(("Received an exorbitant amount of data from the UART!"));
    return false; // something is totally wrong
  }

  length = (uint16_t)count;

  NABTO_LOG_TRACE(("Queued %u bytes in UART driver receive buffer.", length));

  queue_enqueue_array(&uartChannel->receiveQueue, buffer, length);

  return true;
}

static uart_channel* look_up_uart_channel(uint8_t channel)
{
  if (channel >= UART_NUMBER_OF_CHANNELS)
  {
    NABTO_LOG_FATAL(("Invalid UART channel specified!"));
    return NULL;
  }

  return &channels[channel];
}
