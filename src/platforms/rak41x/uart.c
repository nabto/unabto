/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */

#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_PERIPHERAL

#include <unabto/unabto_env_base.h>
#include "uart.h"
#include <unabto/util/unabto_queue.h>
#include <nano1xx_uart.h>

static bool initialized = false;

static queue_t receiveQueue;
static queue_t transmitQueue;

static uint8_t receiveQueueBuffer[NABTO_UART_RECEIVE_BUFFER_SIZE];
static uint8_t transmitQueueBuffer[NABTO_UART_TRANSMIT_BUFFER_SIZE];
static bool receiveQueueOverrun;

void uart_initialize(uint32_t baudrate)
{
  queue_init(&receiveQueue, receiveQueueBuffer, sizeof (receiveQueueBuffer));
  queue_init(&transmitQueue, transmitQueueBuffer, sizeof (transmitQueueBuffer));

  receiveQueueOverrun = false;
  
  SYS_SelectIPClockSource_1(CLK_CLKSEL1_UART_MASK, CLK_CLKSEL1_UART_HXT); // Select UART Clock Source From 12Mhz
  
  MFP_UART0_TO_PORTB();
  
  {
    STR_UART_T sParam;
    sParam.u32BaudRate = baudrate;
    sParam.u32cDataBits = DRVUART_DATABITS_8;
    sParam.u32cStopBits = DRVUART_STOPBITS_1;
    sParam.u32cParity = DRVUART_PARITY_NONE;
    sParam.u32cRxTriggerLevel = DRVUART_FIFO_8BYTES;
    sParam.u8EnableDiv16 = DISABLE;
  
    UART_Init(UART0, &sParam);
  }
  
  UART_EnableInt(UART0, DRVUART_RDAINT); // Enable UART interrupts
  
  initialized = true;
}

bool uart_overrun(bool* queueOverrun)
{
  bool value;

  critical_section_enter();

  *queueOverrun = receiveQueueOverrun;
  value = receiveQueueOverrun;
  receiveQueueOverrun = false;

  critical_section_exit();

  return value;
}

bool uart_framing_error(void)
{
  return false;
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
  
  if(initialized == false) // dump if UART is not initialized
  {
    return;
  }

  do
  {
    critical_section_enter();
    queued = queue_enqueue(&transmitQueue, value);
    UART0->IER |= DRVUART_THREINT; // Enable TX interrupt
    critical_section_exit();
  } while(queued == false);
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
  queue_reset(&receiveQueue);
  receiveQueueOverrun = false;
}

void UART0_IRQHandler(void)
{
  // Service receiver
  while ((UART0->FSR & UART_FSR_RX_EMPTY_F) == 0) // while UART RX FIFO is not empty
  {
    if(queue_enqueue(&receiveQueue, UART0->RBR) == false)
    {
      receiveQueueOverrun = true;
    }
  }
  
  // Service transmitter
  while((UART0->FSR & UART_FSR_TX_FULL_F) == 0) // while UART TX FIFO not full
  {
    uint8_t value;
    
    if(queue_dequeue(&transmitQueue, &value)) // if there was a byte to send
    {
      UART0->THR = value; // move to FIFO
    }
    else
    {
      UART0->IER &= ~DRVUART_THREINT; // Disable TX interrupt
      break;
    }
  }
}
