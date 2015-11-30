/*
* Copyright (C) 2008-2014 Nabto - All Rights Reserved.
*/

#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_NSLP

#include <string.h>
#include <unabto/unabto.h>
#include <unabto/unabto_util.h>
#include "nslp_definitions.h"
#include "nslp_gateway.h"
#include "nslp_binary_link.h"
#include "nslp_binary_transport.h"
#include "nslp_streaming.h"

// NSLP configuration parameters
bool nslpConfigurationShowClientId = true;

void nslp_initialize(void)
{
  NABTO_LOG_TRACE(("Initializing."));

  nslp_binary_transport_initialize();

#if NABTO_ENABLE_STREAM
  nslp_streaming_initialize();
#endif
}

void nslp_tick(void)
{
  nslp_binary_transport_tick();

#if NABTO_ENABLE_STREAM
  nslp_streaming_tick();
#endif
}

bool nslp_gateway_send_notification(uint16_t id, void* arguments, uint16_t argumentsLength)
{
  NABTO_LOG_TRACE(("Sending NSLP notification."));

  if (nslp_binary_transport_allocate_packet(1 + 2 + argumentsLength))
  {
    nslp_binary_transport_write_uint8(NSLP_COMMAND_NOTIFICATION);
    nslp_binary_transport_write_uint16(id);
    nslp_binary_transport_write_buffer(arguments, argumentsLength);

    return true;
  }
  else
  {
    return false;
  }
}

bool nslp_gateway_send_query_request(uint8_t handle, uint32_t flags, const char* clientId, uint16_t responseCapacity, uint32_t queryId, const uint8_t* parameters, uint16_t parametersLength)
{
  uint16_t lengthOfClientId; // number of bytes in an empty string is 1 (the zero-termination)

  NABTO_LOG_TRACE(("Sending NSLP query request."));

  if (nslpConfigurationShowClientId) // if configured to show client id...
  {
    flags |= NSLP_COMMAND_QUERY_REQUEST_FLAG_CLIENT_ID_PRESENT;
    lengthOfClientId = strlen(clientId) + 1; // length of client id and zero-termination
  }
  else
  {
    lengthOfClientId = 0; // no client id
  }

  flags |= NSLP_COMMAND_QUERY_REQUEST_FLAG_RESPONSE_CAPACITY_PRESENT;

  if (nslp_binary_transport_allocate_packet(1 + 4 + lengthOfClientId + 2 + 4 + parametersLength))
  {
    nslp_binary_transport_write_uint8(NSLP_COMMAND_QUERY_REQUEST);

    nslp_binary_transport_write_uint32(flags);

    if (flags & NSLP_COMMAND_QUERY_REQUEST_FLAG_CLIENT_ID_PRESENT)
    {
      nslp_binary_transport_write_buffer(clientId, lengthOfClientId); // insert client id and zero-termination
    }

    if (flags & NSLP_COMMAND_QUERY_REQUEST_FLAG_RESPONSE_CAPACITY_PRESENT)
    {
      nslp_binary_transport_write_uint16(responseCapacity);
    }

    nslp_binary_transport_write_uint32(queryId);

    nslp_binary_transport_write_buffer(parameters, parametersLength);

    return true;
  }
  else
  {
    return false;
  }
}

//bool nslp_gateway_send_response_capacity(uint16_t capacity)
//{
//  NABTO_LOG_TRACE(("Sending NSLP query response capacity."));
//
//  if (nslp_binary_transport_allocate_packet(1 + 2))
//  {
//    nslp_binary_transport_write_uint8(NSLP_COMMAND_QUERY_RESPONSE_CAPACITY);
//    nslp_binary_transport_write_uint16(capacity);
//
//    return true;
//  }
//  else
//  {
//    return false;
//  }
//}

bool nslp_gateway_send_query_drop(uint8_t handle)
{
  NABTO_LOG_TRACE(("Sending NSLP query drop."));

  if (nslp_binary_transport_allocate_packet(1 + 1))
  {
    nslp_binary_transport_write_uint8(NSLP_COMMAND_QUERY_DROP);
    nslp_binary_transport_write_uint8(handle);

    return true;
  }
  else
  {
    return false;
  }
}