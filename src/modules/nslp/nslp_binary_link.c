/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_NSLP

#include "nslp_binary_link.h"
#include "nslp_binary_transport.h"
#include "nslp_communications_port.h"
#include <unabto/unabto_env_base.h>

#define END                                                                   0xc0
#define ESC                                                                   0xdb
#define ESC_END                                                               0xdc
#define ESC_ESC                                                               0xdd

#define NSLP_BINARY_TRANSPORT_MAXIMUM_PACKET_SIZE                             1 + NSLP_MAXIMUM_COMMAND_PARAMETERS_SIZE
#define NSLP_BINARY_LINK_MAXIMUM_FRAME_SIZE                                   ((NSLP_BINARY_TRANSPORT_PACKET_OVERHEAD) + (NSLP_BINARY_TRANSPORT_MAXIMUM_PACKET_SIZE))

static void reset_receiver(void);
static void store_byte(uint8_t value);

#if UNABTO_PLATFORM_PIC18
#pragma udata big_mem
#endif

static uint8_t receiveFrameBuffer[NSLP_BINARY_LINK_MAXIMUM_FRAME_SIZE];

#if UNABTO_PLATFORM_PIC18
#pragma udata
#endif

static uint16_t receiveFrameLength;
static bool endReceived = false;
static bool escReceived;

void nslp_binary_link_start_frame(void)
{
  nslp_communications_port_write(END);
}

void nslp_binary_link_write_to_frame(uint8_t value)
{
  switch(value)
  {
    case END:
      nslp_communications_port_write(ESC);
      nslp_communications_port_write(ESC_END);
      break;

    case ESC:
      nslp_communications_port_write(ESC);
      nslp_communications_port_write(ESC_ESC);
      break;

    default:
      nslp_communications_port_write(value);
      break;
  }
}

void nslp_binary_link_end_frame(void)
{
  nslp_communications_port_write(END);
}

uint16_t nslp_binary_link_receive_frame(uint8_t** frame)
{
  while(nslp_communications_port_can_read())
  {
    uint8_t value = nslp_communications_port_read();

    if(value >= ' ')
    {
      NABTO_LOG_TRACE(("Received %-3u %c", (int) value, value));
    }
    else
    {
      NABTO_LOG_TRACE(("Received %-3u", (int) value));
    }

    switch(value)
    {
      case END:
        NABTO_LOG_TRACE(("END end=%u esc=%u length=%u", endReceived, escReceived, receiveFrameLength));

        if(escReceived)
        {
          reset_receiver();

          endReceived = true;
        }
        else
        {
          if(endReceived)
          {
            uint16_t length = receiveFrameLength; // save frame length before resetting receiver

            NABTO_LOG_TRACE(("frame: length=%u", (int) length));

            reset_receiver();

            if(length > 0)
            {
              NABTO_LOG_TRACE(("Received frame (length=%u).", (int) length));
              NABTO_LOG_BUFFER(NABTO_LOG_SEVERITY_TRACE, ("Received NSLP link layer frame:"), receiveFrameBuffer, length);

              *frame = receiveFrameBuffer;

              return length;
            }
          }
          else
          {
            reset_receiver();

            endReceived = true;
          }
        }
        break;

      case ESC:
        NABTO_LOG_TRACE(("ESC"));

        if(endReceived)
        {
          if(escReceived == false)
          {
            escReceived = true;
          }
          else
          {
            reset_receiver();
          }
        }
        break;

      case ESC_END:
        NABTO_LOG_TRACE(("ESC_END"));

        if(endReceived)
        {
          if(escReceived)
          {
            escReceived = false;
            store_byte(END);
          }
          else
          {
            store_byte(ESC_END);
          }
        }
        break;

      case ESC_ESC:
        NABTO_LOG_TRACE(("ESC_ESC"));

        if(endReceived)
        {
          if(escReceived)
          {
            escReceived = false;
            store_byte(ESC);
          }
          else
          {
            store_byte(ESC_ESC);
          }
        }
        break;

      default:
        NABTO_LOG_TRACE(("data %u", (int) value));

        if(endReceived)
        {
          if(escReceived == false)
          {
            store_byte(value);
          }
          else
          {
            reset_receiver();
          }
        }
        break;
    }
  }

  return 0;
}

static void reset_receiver(void)
{
  receiveFrameLength = 0;
  endReceived = false;
  escReceived = false;
  NABTO_LOG_TRACE(("reset_receiver()"));
}

static void store_byte(uint8_t value)
{
  if(receiveFrameLength < sizeof (receiveFrameBuffer))
  {
    receiveFrameBuffer[receiveFrameLength++] = value;
    NABTO_LOG_TRACE(("store_byte(%u)", (int) value));
  }
  else
  {
    reset_receiver();
  }
}
