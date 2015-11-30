/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_NSLP

#include <string.h>
#include <unabto/unabto.h>
#include <unabto/unabto_util.h>
#include <unabto/unabto_stream.h>
#include "nslp_definitions.h"
#include "nslp_streaming.h"
#include "nslp_gateway.h"
#include "nslp_binary_transport.h"
#include "nslp_communications_port.h"

#if !NABTO_ACL_ENABLE
#define ACL_PERMISSION_NONE 1
#define acl_is_request_allowed(request, permissions) (true)
#define acl_is_stream_allowed(streamHandle, permissions) (true)

#define acl_application_event(request, readBuffer, writeBuffer) (AER_REQ_INV_QUERY_ID)
#endif

#if NABTO_ENABLE_STREAM

#if NSLP_GATEWAY

uint32_t uartQuietTime;

extern char nabtoGeneralPurposeBuffer[NABTO_GENERAL_PURPOSE_BUFFER_SIZE];

//#if NABTO_ENABLE_CLIENT_ID != 1
//#error ERROR: You must enable client identification for NSLP_GATEWAY to work
//#endif

#include "application.h"

typedef enum
{
  NSLP_STREAM_STATE_UNUSED,
  NSLP_STREAM_STATE_ACCEPTED,
  NSLP_STREAM_STATE_OPENING1,
  NSLP_STREAM_STATE_OPENING2,
  NSLP_STREAM_STATE_OPENED,
  NSLP_STREAM_STATE_CLOSING
} nslp_stream_state;

struct nabto_stream_s;

typedef struct
{
  nslp_stream_state state;
  char serviceConfigurationString[NSLP_MAXIMUM_CONFIG_STRING];
  uint8_t serviceConfigurationStringLength;
  uint8_t service;
  struct nabto_stream_s* handle;
  nabto_stamp_t timestamp;
  uint16_t previousLength;
  uint32_t totalReceived;
  uint32_t totalTransmitted;
} nslp_stream;

static nslp_stream streams[NABTO_STREAM_MAX_STREAMS];

#endif

void nslp_streaming_initialize(void)
{
#if NSLP_GATEWAY
  memset(streams, 0, sizeof (streams));
#endif

  NABTO_LOG_TRACE(("Initialized."));
}

void unabto_stream_accept(unabto_stream* uNabtoStream)
{
  uint8_t i;

  for(i = 0; i < NABTO_STREAM_MAX_STREAMS; i++)
  {
    nslp_stream* nslpStream = &streams[i];

    if(nslpStream->state == NSLP_STREAM_STATE_UNUSED)
    {
      NABTO_LOG_TRACE(("Stream accepted [%" PRIu8 "].", i));
      memset(nslpStream, 0, sizeof (nslp_stream));
      nslpStream->state = NSLP_STREAM_STATE_ACCEPTED;
      nslpStream->handle = uNabtoStream;
      nabtoSetFutureStamp(&nslpStream->timestamp, 0);
      return;
    }
  }

  NABTO_LOG_ERROR(("No resources free to accept new stream"));
  unabto_stream_release(uNabtoStream);
}

void nslp_streaming_tick(void)
{
#if NSLP_GATEWAY
  uint8_t errorReply[] = {'-', '\n'};
  uint8_t okReply[] = {'+', '\n'};
  uint8_t i;
  unabto_stream_hint hint;

  // tick all streams
  for(i = 0; i < NABTO_STREAM_MAX_STREAMS; i++)
  {
    nslp_stream* stream = &streams[i];
    uint8_t oldState = stream->state;

    switch(stream->state)
    {
      case NSLP_STREAM_STATE_UNUSED:
        break;

      case NSLP_STREAM_STATE_ACCEPTED:
      {
        uint8_t* p;
        uint16_t length = unabto_stream_read(stream->handle, (const uint8_t**) &p, &hint);

        if(length > 0)
        {
          unabto_stream_ack(stream->handle, p, 1, &hint);

          if(*p == '\r')
          {
            // just ignore these
          }
          else if(*p != '\n') // add char to configuration string
          {
            if(stream->serviceConfigurationStringLength < (NSLP_MAXIMUM_CONFIG_STRING - 1))
            {
              stream->serviceConfigurationString[stream->serviceConfigurationStringLength++] = *p;
            }
            else
            {
              NABTO_LOG_TRACE(("Service configuration string too long [%" PRIu8 "].", i));
              unabto_stream_write(stream->handle, errorReply, sizeof (errorReply), &hint);
              stream->state = NSLP_STREAM_STATE_CLOSING;
            }
          }
          else
          {
            if(textcmp("echo", stream->serviceConfigurationString) == 0 && acl_is_stream_allowed(stream->handle, ACL_PERMISSION_NONE))
            {
              if(unabto_stream_write(stream->handle, okReply, sizeof (okReply), &hint) == sizeof (okReply))
              {
                stream->service = NSLP_STREAM_SERVICE_ECHO;
                stream->state = NSLP_STREAM_STATE_OPENED;
              }
            }
              //            else if(compare_string_and_buf("muxstream", stream->serviceConfigurationString) == 0 && acl_is_stream_allowed(stream->handle, PERMISSION_MULTIPLEXED_STREAM))
              //            {
              //              if(nabto_stream_write(stream->handle, okReply, sizeof (okReply), &hint) == sizeof (okReply))
              //              {
              //                stream->service = NSLP_STREAM_SERVICE_MULTIPLEXED;
              //                stream->state = NSLP_STREAM_STATE_CLOSING;
              //              }
              //            }
            else if(textcmp("nonmuxstream", stream->serviceConfigurationString) == 0 && acl_is_stream_allowed(stream->handle, PERMISSION_NON_MULTIPLEXED_STREAM))
            {
              if(unabto_stream_write(stream->handle, okReply, sizeof (okReply), &hint) == sizeof (okReply))
              {
                stream->service = NSLP_STREAM_SERVICE_NON_MULTIPLEXED;

                nslpCommunicationsPortLock = true;

                stream->state = NSLP_STREAM_STATE_OPENING1;
              }
            }
            else if(textcmp("forcednonmuxstream", stream->serviceConfigurationString) == 0 && acl_is_stream_allowed(stream->handle, PERMISSION_FORCED_NON_MULTIPLEXED_STREAM))
            {
              if(unabto_stream_write(stream->handle, okReply, sizeof (okReply), &hint) == sizeof (okReply))
              {
                stream->service = NSLP_STREAM_SERVICE_FORCED_NON_MULTIPLEXED;

                nslpCommunicationsPortLock = true;

                // abort all current transactions
                nslp_binary_transport_reset();

                stream->state = NSLP_STREAM_STATE_OPENED;
              }
            }

            if(stream->state != NSLP_STREAM_STATE_ACCEPTED)
            {
              NABTO_LOG_TRACE(("Opened %s stream [%" PRIu8 "].", stream->serviceConfigurationString, i));
            }
            else
            {
              NABTO_LOG_TRACE(("Rejecting stream service request: %s [%" PRIu8 "].", stream->serviceConfigurationString, i));
              unabto_stream_write(stream->handle, errorReply, sizeof (errorReply), &hint);
              stream->state = NSLP_STREAM_STATE_CLOSING;
            }
          }
        }
        else if(hint != 0)
        {
          NABTO_LOG_TRACE(("Stream closed prior to receiving configuration string (hint=%i) [%" PRIu8 "].", hint, i));
          stream->state = NSLP_STREAM_STATE_CLOSING;
        }
      }
        break;

      case NSLP_STREAM_STATE_OPENING1:
      {
        switch(stream->service)
        {
          case NSLP_STREAM_SERVICE_MULTIPLEXED:
            break;

          case NSLP_STREAM_SERVICE_NON_MULTIPLEXED:
            if(nslp_gateway_send_notification(NSLP_NOTIFICATION_NON_MULTIPLEXED_STREAM_OPENED, NULL, 0))
            {
              NABTO_LOG_TRACE(("Sending 'non mux stream' notification [%" PRIu8 "].", i));
              stream->state = NSLP_STREAM_STATE_OPENING2;
            }
            else
            {
              NABTO_LOG_TRACE(("Unable send stream open notification to NSLP client [%" PRIu8 "].", i));
              stream->state = NSLP_STREAM_STATE_CLOSING;
            }
            break;
        }
      }
        break;

      case NSLP_STREAM_STATE_OPENING2:
      {
        switch(stream->service)
        {
          case NSLP_STREAM_SERVICE_MULTIPLEXED:
            break;

          case NSLP_STREAM_SERVICE_NON_MULTIPLEXED:
            // wait for notification to be sent before actually opening the stream
            if(nslp_binary_transport_is_idle())
            {
              NABTO_LOG_TRACE(("Sent 'non mux stream' notification [%" PRIu8 "].", i));
              stream->state = NSLP_STREAM_STATE_OPENED;
            }
            break;
        }
      }
        break;

      case NSLP_STREAM_STATE_OPENED:
      {
        uint8_t* p;
        uint16_t length;

        switch(stream->service)
        {
          case NSLP_STREAM_SERVICE_ECHO:
            length = unabto_stream_read(stream->handle, (const uint8_t**) &p, &hint);
            if(length > 0)
            {
              length = unabto_stream_write(stream->handle, p, length, &hint);
              if(length > 0)
              {
                if(unabto_stream_ack(stream->handle, p, length, &hint) == false || hint != 0)
                {
                  NABTO_LOG_TRACE(("Unable to acknowledge data from stream [%" PRIu8 "] hint=%i.", i, hint));
                  stream->state = NSLP_STREAM_STATE_CLOSING;
                }
              }
              else if(hint != 0)
              {
                NABTO_LOG_TRACE(("Unable to write to stream [%" PRIu8 "] hint=%i.", i, hint));
                stream->state = NSLP_STREAM_STATE_CLOSING;
              }
            }
            else if(hint != 0)
            {
              NABTO_LOG_TRACE(("Unable to read from stream [%" PRIu8 "] hint=%i.", i, hint));
              stream->state = NSLP_STREAM_STATE_CLOSING;
            }
            break;

          case NSLP_STREAM_SERVICE_MULTIPLEXED:
            break;

          case NSLP_STREAM_SERVICE_NON_MULTIPLEXED:
          case NSLP_STREAM_SERVICE_FORCED_NON_MULTIPLEXED:
            // copy from stream to communications port
            length = unabto_stream_read(stream->handle, (const uint8_t**) &p, &hint);
            if(length > 0)
            {
              uint16_t length2 = nslp_communications_port_can_write();
              length = MIN(length, length2);
              if(length > 0)
              {
                nslp_communications_port_write_buffer((const void*) p, length);
                if(unabto_stream_ack(stream->handle, p, length, &hint) == false || hint != 0)
                {
                  NABTO_LOG_TRACE(("Unable to acknowledge data from non mux stream [%" PRIu8 "] hint=%i.", i, hint));
                  stream->state = NSLP_STREAM_STATE_CLOSING;
                }
                else
                {
                  stream->totalTransmitted += length;
                  NABTO_LOG_TRACE(("> %" PRIu16 "/%" PRIu32, length, stream->totalTransmitted));
                }
              }
            }
            else if(hint != 0)
            {
              NABTO_LOG_TRACE(("Unable to read from non mux stream [%" PRIu8 "] hint=%i.", i, hint));
              stream->state = NSLP_STREAM_STATE_CLOSING;
            }

            // copy from communications port to stream - only if stream -> communications port didn't fail
            if(stream->state == NSLP_STREAM_STATE_OPENED)
            {
              bool queueOverrun;
              if(uart_overrun(&queueOverrun))
              {
                NABTO_LOG_TRACE(("UART overrun. queueOverrun: %i", queueOverrun));
              }

              // copy from communications port to stream
              length = nslp_communications_port_can_read();
              if(length == 0)
              {
                stream->previousLength = 0;
                nabtoSetFutureStamp(&stream->timestamp, uartQuietTime);
              }
              else if(length >= NABTO_STREAM_SEND_SEGMENT_SIZE || nabtoIsStampPassed(&stream->timestamp) || uartQuietTime == 0) // UART has a full segment to send or quite time expired or quite time disabled -> send
              {
                length = MIN(length, unabto_stream_can_write(stream->handle, &hint)); // bytes that can be moved from UART stream to Nabto stream
                if(length > 0)
                {
                  length = MIN(length, sizeof (nabtoGeneralPurposeBuffer)); // bytes to move right now
                  nslp_communications_port_read_buffer(nabtoGeneralPurposeBuffer, length);
                  if(unabto_stream_write(stream->handle, (const uint8_t*) nabtoGeneralPurposeBuffer, length, &hint) != length || hint != 0)
                  {
                    NABTO_LOG_TRACE(("Unable to write to non mux stream [%" PRIu8 "] hint=%i.", i, hint));
                    stream->state = NSLP_STREAM_STATE_CLOSING;
                  }
                  else
                  {
                    stream->totalReceived += length;
                    NABTO_LOG_TRACE(("< %" PRIu16 "/%" PRIu32, length, stream->totalReceived));
                  }
                }
                else if(hint != 0)
                {
                  NABTO_LOG_TRACE(("Unable to access stream [%" PRIu8 "] hint=%i.", i, hint));
                  stream->state = NSLP_STREAM_STATE_CLOSING;
                }

                stream->previousLength = nslp_communications_port_can_read();
                nabtoSetFutureStamp(&stream->timestamp, uartQuietTime);
              }
              else if(length != stream->previousLength)
              {
                stream->previousLength = length;
                nabtoSetFutureStamp(&stream->timestamp, uartQuietTime);
              }
            }
            break;
        }
      }
        break;

      case NSLP_STREAM_STATE_CLOSING:
      {
        uint8_t* p;
        uint16_t length;

        // flush stream (incoming data) to allow for a clean close
        length = unabto_stream_read(stream->handle, (const uint8_t**) &p, &hint);
        if(length > 0)
        {
          unabto_stream_ack(stream->handle, p, length, &hint);
        }

        if(unabto_stream_close(stream->handle))
        {
          unabto_stream_release(stream->handle);

          if(stream->service == NSLP_STREAM_SERVICE_NON_MULTIPLEXED || stream->service == NSLP_STREAM_SERVICE_FORCED_NON_MULTIPLEXED)
          {
            nslpCommunicationsPortLock = false;
          }

          NABTO_LOG_TRACE(("Stream closed [%" PRIu8 "].", i));
          stream->state = NSLP_STREAM_STATE_UNUSED;
        }
      }
        break;
    }

    if(oldState != stream->state)
    {
      NABTO_LOG_TRACE(("Stream state changed %u->%u (service=%u) [%" PRIu8 "].", (int) oldState, (int) stream->state, (int) stream->service, i));
      i--; // tick stream object again
    }
  }

#endif
}

#endif
