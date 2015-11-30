/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#ifndef _NSLP_DEFINITIONS_H_
#define _NSLP_DEFINITIONS_H_

/*

  The following detalis all on-wire data structures related to NSLP.


  The table below shows the NSLP application layer protocol packet format:

  +---------+----------------------------------------------------------------------+
  | uint8   | Command.                                                             |
  +---------+----------------------------------------------------------------------+
  | uint8[] | Command parameters.                                                  |
  +---------+----------------------------------------------------------------------+

  The following details each command:

  NSLP_COMMAND_NOTIFICATION
  +---------+----------------------------------------------------------------------+
  | uint8   | Command.                                                             |
  +---------+----------------------------------------------------------------------+
  | uint16  | Notification id.                                                     |
  +---------+----------------------------------------------------------------------+
  | uint8[] | Notification parameters.                                             |
  +---------+----------------------------------------------------------------------+

  NSLP_COMMAND_QUERY_REQUEST
  +---------+----------------------------------------------------------------------+
  | uint8   | Command.                                                             |
  +---------+----------------------------------------------------------------------+
  | uint8   | Handle.                                                              |
  +---------+----------------------------------------------------------------------+
  | uint32  | Flags.                                                               |
  |         |  0 Is local. The request was sent from a locally connected client.   |
  |         |  1 Client id present. The request contains a client id.              |
  |         |  2 Response capacity present. The request contains the current       |
  |         |    response capacity of the gateway.                                 |
  +---------+----------------------------------------------------------------------+
  | char[]  | Client id. Zero-terminated string. Optional.                         |
  +---------+----------------------------------------------------------------------+
  | uint16  | Response capacity. Optional.                                         |
  +---------+----------------------------------------------------------------------+
  | uint32  | Query id.                                                            |
  +---------+----------------------------------------------------------------------+
  | uint8[] | Query parameters.                                                    |
  +---------+----------------------------------------------------------------------+

  NSLP_COMMAND_QUERY_RESPONSE
  +---------+----------------------------------------------------------------------+
  | uint8   | Command.                                                             |
  +---------+----------------------------------------------------------------------+
  | uint8   | Handle.                                                              |
  +---------+----------------------------------------------------------------------+
  | uint8   | Status.                                                              |
  +---------+----------------------------------------------------------------------+
  | uint8[] | Response data.                                                       |
  +---------+----------------------------------------------------------------------+

  NSLP_COMMAND_QUERY_RESPONSE_CAPACITY
  +---------+----------------------------------------------------------------------+
  | uint8   | Command.                                                             |
  +---------+----------------------------------------------------------------------+
  | uint16  | Capacity i.e. number of response bytes the gateway is ready to       |
  |         | received.                                                            |
  +---------+----------------------------------------------------------------------+

  NSLP_COMMAND_QUERY_DROP
  +---------+----------------------------------------------------------------------+
  | uint8   | Command.                                                             |
  +---------+----------------------------------------------------------------------+
  | uint8   | Handle.                                                              |
  +---------+----------------------------------------------------------------------+


  The table below shows the NSLP binary transport protocol packet format:

  +---------+----------------------------------------------------------------------+
  | uint8   | Protocol version.                                                    |
  +---------+----------------------------------------------------------------------+
  | uint16  | Flags:                                                               |
  |         |  0 From gateway. The packet was sent from the gateway.               |
  +---------+----------------------------------------------------------------------+
  | uint8   | Acknowledging sequence number.                                       |
  +---------+----------------------------------------------------------------------+
  | uint8   | Senders sequence number.                                             |
  +---------+----------------------------------------------------------------------+
  | uint8[] | Payload.                                                             |
  +---------+----------------------------------------------------------------------+
  | uint8   | Checksum. One's complement sum.                                      |
  +---------+----------------------------------------------------------------------+


  The table below shows the NSLP binary link protocol packet format:
  This is simply a SLIP framing and byte stuffing of the transport protocol packet.

  +---------+----------------------------------------------------------------------+
  | uint8   | ESC.                                                                 |
  +---------+----------------------------------------------------------------------+
  | uint8[] | Payload - byte stuffed where needed.                                 |
  +---------+----------------------------------------------------------------------+
  | uint8   | ESC.                                                                 |
  +---------+----------------------------------------------------------------------+

  */

typedef enum
{
  // notifications
  NSLP_COMMAND_NOTIFICATION,

  // queries
  NSLP_COMMAND_QUERY_REQUEST,
  NSLP_COMMAND_QUERY_RESPONSE,
  NSLP_COMMAND_QUERY_RESPONSE_CAPACITY,
  NSLP_COMMAND_QUERY_DROP

  // streams
  //  NSLP_COMMAND_MULTIPLEXED_STREAM_REQUEST,
  //  NSLP_COMMAND_MULTIPLEXED_STREAM_RESPONSE,
  //  NSLP_COMMAND_MULTIPLEXED_STREAM_PUSH,

  // event data
  //  NSLP_COMMAND_EVENT_DATA_PUSH,
  //  NSLP_COMMAND_EVENT_DATA_ACKNOWLEDGE
} nslp_command;

typedef enum
{
  NSLP_COMMAND_QUERY_REQUEST_FLAG_NONE = 0,
  NSLP_COMMAND_QUERY_REQUEST_FLAG_IS_LOCAL = 1,
  NSLP_COMMAND_QUERY_REQUEST_FLAG_CLIENT_ID_PRESENT = 2,
  NSLP_COMMAND_QUERY_REQUEST_FLAG_RESPONSE_CAPACITY_PRESENT = 4
} nslp_command_query_request_flag;

typedef enum
{
  NSLP_NOTIFICATION_LINK_UP,
  NSLP_NOTIFICATION_LINK_DOWN,
  NSLP_NOTIFICATION_DHCP_SUCCESS,
  NSLP_NOTIFICATION_GSP_ATTACH,
  NSLP_NOTIFICATION_GSP_DETACH,
  NSLP_NOTIFICATION_EVENT_QUEUE_EMPTY,
  NSLP_NOTIFICATION_NON_MULTIPLEXED_STREAM_OPENED
} nslp_notification;

typedef enum
{
  NSLP_QUERY_STATUS_OK,
  NSLP_QUERY_STATUS_ABORTED,
  NSLP_QUERY_STATUS_NO_ACCESS,
  NSLP_QUERY_STATUS_OUT_OF_RESOURCES,
  NSLP_QUERY_STATUS_INVALID_QUERY,
  NSLP_QUERY_STATUS_SYSTEM_ERROR
} nslp_query_status;

#endif
