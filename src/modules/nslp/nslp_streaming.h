/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#ifndef _NSLP_STREAMING_H_
#define _NSLP_STREAMING_H_

#include <unabto_platform_types.h>

enum
{
  NSLP_STREAM_SERVICE_NONE,
  NSLP_STREAM_SERVICE_ECHO,
  NSLP_STREAM_SERVICE_MULTIPLEXED,
  NSLP_STREAM_SERVICE_NON_MULTIPLEXED,
  NSLP_STREAM_SERVICE_FORCED_NON_MULTIPLEXED
};

#define NSLP_COMMAND_QUERY_REQUEST_FLAG_NONE               0ul
#define NSLP_COMMAND_QUERY_REQUEST_FLAG_IS_LOCAL           1ul

extern uint32_t uartQuietTime;

void nslp_streaming_initialize(void);
void nslp_streaming_tick(void);

#endif
