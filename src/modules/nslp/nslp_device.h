/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#ifndef _NSLP_DEVICE_H_
#define _NSLP_DEVICE_H_

#include <unabto_platform_types.h>
#include <modules/nslp/nslp_definitions.h>

typedef enum
{
  NSLP_QUERY_STATE_IDLE,
  NSLP_QUERY_STATE_ACTIVE,
  NSLP_QUERY_STATE_COMPLETE
} nslp_query_state;

typedef struct
{
  uint32_t queryId;
  const char* clientId;
  bool isLocal;
  nslp_query_state state;
} nslp_query;

//typedef enum
//{
//  NSLP_STATUS_OK,
//  NSLP_STATUS_BUSY,
//  NSLP_STATUS_OUT_OF_RESOURCES
//} nslp_status;

extern void notification(uint16_t id, const void* data);
extern nslp_query_status begin_nslp_query(nslp_query* query, buffer_read_t* request);
extern nslp_query_status end_nslp_query(nslp_query* query, void** responseData, uint16_t* responseDataLength);
extern void drop_nslp_query(nslp_query* query);

void nslp_initialize(void);
void nslp_tick(void);

#endif
