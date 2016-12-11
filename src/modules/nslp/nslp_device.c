/*
* Copyright (C) 2008-2014 Nabto - All Rights Reserved.
*/

#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_NSLP

#include <unabto/unabto.h>
#include <unabto/unabto_util.h>
#include "nslp_device.h"
#include "nslp_binary_link.h"
#include "nslp_binary_transport.h"

#ifndef NSLP_MAXIMUM_CONCURRENT_QUERIES
#define NSLP_MAXIMUM_CONCURRENT_QUERIES 1
#endif

nslp_query queries[NSLP_MAXIMUM_CONCURRENT_QUERIES];
static uint16_t gatewayResponseCapacity;

// NSLP configuration parameters
bool nslpConfigurationShowClientId = true;

void nslp_initialize(void)
{
  memset(queries, 0, sizeof(queries));
  gatewayResponseCapacity = 0;

  nslp_binary_transport_initialize();
}

bool send_query_response(uint8_t handle, nslp_query_status status, uint8_t* responseData, uint16_t responseDataLength)
{
  NABTO_LOG_TRACE(("Sending NSLP query response."));

  if (nslp_binary_transport_allocate_packet(1 + 1 + 1 + responseDataLength))
  {
    nslp_binary_transport_write_uint8(NSLP_COMMAND_QUERY_RESPONSE);
    nslp_binary_transport_write_uint8(handle);
    nslp_binary_transport_write_uint8((uint8_t)status);
    nslp_binary_transport_write_buffer(responseData, responseDataLength);

    return true;
  }
  else
  {
    return false;
  }
}


void handle_command(uint8_t* packet, uint16_t packetLength)
{
  uint8_t command;

  READ_FORWARD_U8(command, packet);
  packetLength -= 1;

  switch (command)
  {
    // handle notification from the gateway
    case NSLP_COMMAND_NOTIFICATION:
    {
      uint16_t notificationId;

      READ_FORWARD_U16(notificationId, packet);

      notification(notificationId, packet);
    }
      break;

      // application events
    case NSLP_COMMAND_QUERY_REQUEST:
    {
      nslp_query* query;

      for (query = queries; query < (queries + NSLP_MAXIMUM_CONCURRENT_QUERIES); query++) // find idle query slot
      {
        if (query->state == NSLP_QUERY_STATE_IDLE)
        {
          uint8_t* p = packet;
          uint32_t flags;
          uint16_t readBufferLength;
          unabto_buffer readBufferStore;
          unabto_qury_request readBuffer;
          nslp_query_status status;

          memset(query, 0, sizeof(nslp_query));

          READ_FORWARD_U32(flags, p);
          if (flags & NSLP_COMMAND_QUERY_REQUEST_FLAG_IS_LOCAL)
          {
            query->isLocal = true;
          }

          if (flags & NSLP_COMMAND_QUERY_REQUEST_FLAG_CLIENT_ID_PRESENT)
          {
            query->clientId = (const char*)p; // read client id
            while (*p++); // skip past client id and zero-termination
          }

          if (flags & NSLP_COMMAND_QUERY_REQUEST_FLAG_RESPONSE_CAPACITY_PRESENT)
          {
            READ_FORWARD_U16(gatewayResponseCapacity, p);
          }

          READ_FORWARD_U32(query->queryId, p);

          readBufferLength = packetLength - (p - packet);
          buffer_init(&readBufferStore, p, readBufferLength);
          unabto_query_request_init(&readBuffer, &readBufferStore);

          query->state = NSLP_QUERY_STATE_ACTIVE;

          status = begin_nslp_query(query, &readBuffer); // call application query handler

          query->clientId = NULL; // only valid during the NSLP_COMMAND_QUERY_REQUEST so clear it now

          send_query_response(0, status, NULL, 0);

          if (status != NSLP_QUERY_STATUS_OK)
          {
            query->state = NSLP_QUERY_STATE_IDLE;
          }
          break;
        }
      }
    }
      break;

    case NSLP_COMMAND_QUERY_RESPONSE_CAPACITY:
      break;

    case NSLP_COMMAND_QUERY_DROP:
      break;
  }
}

void nslp_tick(void)
{
  uint8_t* packet;
  uint16_t packetLength;
  nslp_query* query;

  nslp_binary_transport_tick();

  packetLength = nslp_binary_transport_receive(&packet);
  if (packetLength > 0)
  {
    handle_command(packet, packetLength);
  }

  for (query = queries; query < (queries + NSLP_MAXIMUM_CONCURRENT_QUERIES); query++)
  {
    if (query->state == NSLP_QUERY_STATE_COMPLETE)
    {
      uint8_t* responseData = NULL;
      uint16_t responseDataLength = 0;
      nslp_query_status status;

      status = end_nslp_query(query, &responseData, &responseDataLength);

      send_query_response(0, status, responseData, responseDataLength);

      query->state = NSLP_QUERY_STATE_IDLE;
    }
  }
}
