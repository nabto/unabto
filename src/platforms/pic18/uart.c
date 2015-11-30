/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */

#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_PERIPHERAL

#include <unabto/unabto_env_base.h>
#include "uart.h"

#if UART_USE_NEW_DRIVER || 1

#if NABTO_UART_RECEIVE_BUFFER_SIZE > 255
#pragma udata big_mem
static uint8_t receiveBuffer[NABTO_UART_RECEIVE_BUFFER_SIZE];
#pragma udata
typedef uint16_t receive_int;
#else
#pragma udata uart_receive_buffer_section
static uint8_t receiveBuffer[NABTO_UART_RECEIVE_BUFFER_SIZE];
#pragma udata
typedef uint8_t receive_int;
#endif

#if NABTO_UART_TRANSMIT_BUFFER_SIZE > 255
#pragma udata big_mem
static uint8_t transmitBuffer[NABTO_UART_TRANSMIT_BUFFER_SIZE];
#pragma udata
typedef uint16_t transmit_int;
#else
#pragma udata uart_transmit_buffer_section
static uint8_t transmitBuffer[NABTO_UART_TRANSMIT_BUFFER_SIZE];
#pragma udata
typedef uint8_t transmit_int;
#endif

#pragma udata access uart_interrupt_access_section

static near receive_int receiveHead;
static near receive_int receiveTail;
static near receive_int receiveCount;
static near transmit_int transmitHead;
static near transmit_int transmitTail;
static near transmit_int transmitCount;
static near bool overrun;
static near bool receiveQueueOverrun;
static near bool framingError;

#pragma udata

void uart_initialize(uint32_t baudrate)
{
  uint16_t temp;

  receiveHead = 0;
  receiveTail = 0;
  receiveCount = 0;
  transmitHead = 0;
  transmitTail = 0;
  transmitCount = 0;
  overrun = false;
  receiveQueueOverrun = false;
  framingError = false;

  TXSTA1 = 0x24;
  RCSTA1 = 0x90;
  BAUDCON1 = 0x08;

  temp = GetPeripheralClock() / baudrate;
  temp--;
  SPBRG1 = (uint8_t) temp;
  SPBRGH1 = (uint8_t) (temp >> 8);

  PIR1bits.RC1IF = 0; // clear the receive interrupt flag
  PIE1bits.RC1IE = 1; // enable the receive interrupt

  TRISCbits.TRISC6 = 1;
}

bool uart_overrun(bool* queueOverrun)
{
  bool value;

  critical_section_enter();

  *queueOverrun = receiveQueueOverrun;
  value = overrun | receiveQueueOverrun;
  overrun = false;
  receiveQueueOverrun = false;

  critical_section_exit();

  return value;
}

bool uart_framing_error(void)
{
  bool value;

  critical_section_enter();

  value = framingError;
  framingError = false;

  critical_section_exit();

  return value;
}

uint16_t uart_can_read(void)
{
#if NABTO_UART_RECEIVE_BUFFER_SIZE > 255
  uint16_t count;

  critical_section_enter();

  count = receiveCount;

  critical_section_exit();

  return count;
#else
  return (uint16_t) receiveCount;
#endif
}

uint16_t uart_can_write(void)
{
  uint16_t count;

  critical_section_enter();

  count = sizeof (transmitBuffer) - transmitCount;

  critical_section_exit();

  return count;
}

uint8_t uart_read(void)
{
  uint8_t value;

  while(uart_can_read() == 0);

  {
    critical_section_enter();

    value = receiveBuffer[receiveTail++];
    receiveTail %= sizeof (receiveBuffer);
    receiveCount--;

    critical_section_exit();
  }

  return value;
}

void uart_read_buffer(void* buffer, uint16_t length)
{
  uint8_t* b = (uint8_t*) buffer;

  while(length--)
  {
    *b++ = uart_read();
  }
}

void uart_write(uint8_t value)
{
  while(uart_can_write() == 0);

  {
    critical_section_enter();

    transmitBuffer[transmitHead++] = value;
    transmitHead %= sizeof (transmitBuffer);
    transmitCount++;

    critical_section_exit();
  }

  PIE1bits.TX1IE = 1; // in case the interrupt wasn't already enabled it should be now
}

void uart_write_buffer(const void* buffer, uint16_t length)
{
  uint8_t* b = (uint8_t*) buffer;

  while(length--)
  {
    uart_write(*b++);
  }
}

void uart_flush_receiver(void)
{
  critical_section_enter();

  receiveHead = 0;
  receiveTail = 0;
  receiveCount = 0;
  overrun = false;
  receiveQueueOverrun = false;
  framingError = false;

  critical_section_exit();
}

void uart_interrupt_service_handler(void)
{
  // service receiver
  while(PIR1bits.RC1IF)
  {
    if(RCSTAbits.FERR1)
    {
      framingError = true;
    }

    if(RCSTAbits.OERR1)
    {
      RCSTAbits.CREN1 = 0;
      RCSTAbits.CREN1 = 1;

      overrun = true;
    }

    if(receiveCount < sizeof (receiveBuffer))
    {
      receiveBuffer[receiveHead++] = RCREG1;
      receiveHead %= sizeof (receiveBuffer);
      receiveCount++;
    }
    else
    {
      WREG = RCREG1;
      receiveQueueOverrun = true;
    }
  }

  // service transmitter
  while(PIE1bits.TX1IE && PIR1bits.TX1IF)
  {
    if(transmitCount > 0)
    {
      TXREG1 = transmitBuffer[transmitTail++];
      transmitTail %= sizeof (transmitBuffer);
      transmitCount--;
    }
    else
    {
      PIE1bits.TX1IE = 0;
    }
  }
}

#else

#include <unabto/util/unabto_queue.h>

static queue_t receiveQueue;
static queue_t transmitQueue;

#pragma udata big_mem
static uint8_t receiveQueueBuffer[NABTO_UART_RECEIVE_BUFFER_SIZE];
static uint8_t transmitQueueBuffer[NABTO_UART_TRANSMIT_BUFFER_SIZE];
#pragma udata

#pragma udata access uart_interrupt_access_section
static near bool overrun;
static near bool receiveQueueOverrun;
static near bool framingError;
static near uint8_t tempValue;
#pragma udata

void uart_initialize(uint32_t baudrate)
{
  uint16_t temp;

  queue_init(&receiveQueue, receiveQueueBuffer, sizeof (receiveQueueBuffer));
  queue_init(&transmitQueue, transmitQueueBuffer, sizeof (transmitQueueBuffer));

  overrun = false;
  receiveQueueOverrun = false;
  framingError = false;

  TXSTA1 = 0x24;
  RCSTA1 = 0x90;
  BAUDCON1 = 0x08;

  temp = GetPeripheralClock() / baudrate;
  temp--;
  SPBRG1 = (uint8_t) temp;
  SPBRGH1 = (uint8_t) (temp >> 8);

  PIR1bits.RC1IF = 0; // clear the receive interrupt flag
  PIE1bits.RC1IE = 1; // enable the receive interrupt

  TRISCbits.TRISC6 = 1;
}

bool uart_overrun(bool* queueOverrun)
{
  bool value;

  critical_section_enter();

  *queueOverrun = receiveQueueOverrun;
  value = overrun | receiveQueueOverrun;
  overrun = false;
  receiveQueueOverrun = false;

  critical_section_exit();

  return value;
}

bool uart_framing_error(void)
{
  bool value;

  critical_section_enter();

  value = framingError;
  framingError = false;

  critical_section_exit();

  return value;
}

uint16_t uart_can_read(void)
{
  uint16_t count;

  critical_section_enter();

  count = receiveQueue.count;

  critical_section_exit();

  return count;
}

uint16_t uart_can_write(void)
{
  uint16_t count;

  critical_section_enter();

  count = transmitQueue.capacity - transmitQueue.count;

  critical_section_exit();

  return count;
}

uint8_t uart_read(void)
{
  bool dequeued;
  uint8_t value;

  do
  {
    critical_section_enter();
    dequeued = queue_dequeue(&receiveQueue, &value);
    critical_section_exit();
  } while(dequeued == false);

  return value;
}

void uart_read_buffer(void* buffer, uint16_t length)
{
  uint8_t* b = (uint8_t*) buffer;
  while(length--)
  {
    *b++ = uart_read();
  }
}

void uart_write(uint8_t value)
{
  bool queued;

  do
  {
    critical_section_enter();
    queued = queue_enqueue(&transmitQueue, value);
    critical_section_exit();
  } while(queued == false);

  PIE1bits.TX1IE = 1; // in case the interrupt wasn't already enabled it should be now
}

void uart_write_buffer(const void* buffer, uint16_t length)
{
  uint8_t* b = (uint8_t*) buffer;
  while(length--)
  {
    uart_write(*b++);
  }
}

void uart_flush_receiver(void)
{
  while(uart_can_read())
  {
    uart_read();
  }

  overrun = false;
  receiveQueueOverrun = false;
  framingError = false;
}

void uart_interrupt_service_handler(void)
{
  // service receiver
  while(PIR1bits.RC1IF)
  {
    if(RCSTAbits.FERR1)
    {
      framingError = true;
    }

    if(RCSTAbits.OERR1)
    {
      RCSTAbits.CREN1 = 0;
      RCSTAbits.CREN1 = 1;

      overrun = true;
    }

    //    tempValue = RCREG1;
    if(queue_enqueue(&receiveQueue, RCREG1) == false)
    {
      receiveQueueOverrun = true;
    }
  }

  // service transmitter
  while(PIE1bits.TX1IE && PIR1bits.TX1IF)
  {
    if(queue_dequeue(&transmitQueue, (uint8_t*) & tempValue))
    {
      TXREG1 = tempValue;
    }
    else
    {
      PIE1bits.TX1IE = 0;
    }
  }
}

#endif

// if the debug channel has not been enabled make the UART standard out

#if NABTO_USE_DEBUG_CHANNEL != 1

int _user_putc(char c)
{
  uart_write(c);
  return c;
}

#endif
