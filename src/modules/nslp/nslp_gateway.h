/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#ifndef _NSLP_GATEWAY_H_
#define _NSLP_GATEWAY_H_

#include <unabto_platform_types.h>

#define NSLP_MAXIMUM_RESPONSE_SIZE          70

void nslp_initialize(void);
void nslp_tick(void);

bool nslp_gateway_send_notification(uint16_t id, void* data, uint16_t dataLength);

// queries
bool nslp_gateway_send_query_request(uint8_t handle, uint32_t flags, const char* clientId, uint16_t responseCapacity, uint32_t queryId, const uint8_t* parameters, uint16_t parametersLength);
//bool nslp_gateway_send_response_capacity(uint16_t capacity);
bool nslp_gateway_send_query_drop(uint8_t handle);

// streams
bool nslp_gateway_push_stream_data(uint8_t streamIndex, uint8_t* data, uint16_t length);

#endif
