/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_NETWORK

#include "dhcp_client.h"
#include <unabto/unabto_memory.h>
#include <string.h>

typedef struct
{
  uint8_t opcode;
  uint8_t hardwareType;
  uint8_t hardwareAddressLength;
  uint8_t hops;
  uint32_t transactionIdentifier;
  uint16_t seconds;
  uint16_t flags;
  uint32_t clientIpAddress;
  uint32_t yourIpAddress;
  uint32_t serverIpAddress;
  uint32_t gatewayIpAddress;
  uint8_t clientHardwareAddress[16];
  char hostName[64];
  char bootFileName[128];
  uint32_t magicCookie;
  uint8_t options[308];
} dhcp_message;

#define BOOTP_MESSAGE_TYPE_BOOT_REQUEST                             1
#define BOOTP_MESSAGE_TYPE_BOOT_REPLY                               2

#define BOOTP_HARDWARE_TYPE_ETHERNET                                1

#define BOOTP_FLAG_BROADCAST                                        0x8000
#define BOOTP_FLAG_UNICAST                                          0x0000

#define DHCP_MESSAGE_TYPE_DISCOVER                                  1
#define DHCP_MESSAGE_TYPE_OFFER                                     2
#define DHCP_MESSAGE_TYPE_REQUEST                                   3
#define DHCP_MESSAGE_TYPE_ACKNOWLEDGE                               5

#define BOOTP_OPTION_PADDING                                        0
#define BOOTP_OPTION_SUBNET_MASK                                    1
#define BOOTP_OPTION_ROUTER                                         3
#define BOOTP_OPTION_DOMAIN_NAME_SERVER                             6
#define BOOTP_OPTION_HOST_NAME                                      12
#define BOOTP_OPTION_REQUESTED_IP_ADDRESS                           50
#define BOOTP_OPTION_IP_ADDRESS_LEASE_TIME                          51
#define BOOTP_OPTION_DHCP_MESSAGE_TYPE                              53
#define BOOTP_OPTION_DHCP_SERVER_IDENTIFIER                         54
#define BOOTP_OPTION_PARAMETER_REQUEST_LIST                         55
#define BOOTP_OPTION_END                                            255

#define PARAMETER_REQUEST_ITEM_SUBNET_MASK                          1
#define PARAMETER_REQUEST_ITEM_ROUTER                               3
#define PARAMETER_REQUEST_ITEM_DOMAIN_NAME_SERVER                   6
#define PARAMETER_REQUEST_ITEM_HOST_NAME                            12

#define MINIMUM_TIMEOUT_DURATION                                    4000
#define MAXIMUM_TIMEOUT_DURATION                                    64000

#define IP_BROADCAST_ADDRESS                                        0xffffffff

typedef enum
{
  STATE_INITIALIZING,
  STATE_UNBOUND,
  STATE_WAITING_FOR_OFFER,
  STATE_WAITING_FOR_ACKNOWLEDGE,
  STATE_BOUND,
  STATE_RENEWING
} resolver_state;

dhcp_client_information dhcpClientInformation;

static void send_discover_message(void);
static void send_request_message(uint32_t offeredIp);

static uint8_t hardwareAddress[6];

static nabto_socket_t socket;
static resolver_state state;
static nabto_stamp_t timer;
static nabto_stamp_t leaseExpirationTimer;
static uint32_t timeoutDuration;
static uint32_t transactionIdentifier;

void dhcp_client_initialize(const uint8_t* mac)
{
  memcpy(hardwareAddress, mac, sizeof (hardwareAddress));

  memset(&dhcpClientInformation, 0, sizeof (dhcpClientInformation));

  state = STATE_INITIALIZING;

  // On some network interfaces the first socket allocated has special
  // abilities needed for DHCP to work so give the DHCP state machine
  // an early tick to acquire this (possibly) special socket.
  dhcp_client_tick();
}

void dhcp_client_tick(void)
{
  switch(state)
  {
    case STATE_INITIALIZING:
    {
      uint16_t localPort = 68;
      if(nabto_init_socket(0, &localPort, &socket))
      {
        state = STATE_UNBOUND;
        timeoutDuration = MINIMUM_TIMEOUT_DURATION;
        NABTO_LOG_TRACE(("Opened socket."));
      }
    }
      break;

    case STATE_UNBOUND:
    {
      send_discover_message();

      state = STATE_WAITING_FOR_OFFER;
      nabtoSetFutureStamp(&timer, timeoutDuration);

      NABTO_LOG_TRACE(("Requesting lease."));
    }
      break;

    case STATE_WAITING_FOR_OFFER:
    {
      uint32_t sourceIp;
      uint16_t sourcePort;
      uint16_t length = nabto_read(socket, nabtoCommunicationBuffer, sizeof (nabtoCommunicationBuffer), &sourceIp, &sourcePort);

      if(length > 0)
      {
        uint8_t dhcpMessageType;
        uint32_t offeredIp;
        dhcp_message* message = (dhcp_message*) nabtoCommunicationBuffer;
        uint8_t* p = message->options;

        READ_U32(offeredIp, &message->yourIpAddress);

        while(p < (message->options + sizeof (message->options))) // go through entire options block
        {
          switch(*p++)
          {
            case BOOTP_OPTION_PADDING: // ignoring padding bytes
              break;

            case BOOTP_OPTION_DHCP_MESSAGE_TYPE:
              p++;
              dhcpMessageType = *p++;
              break;

            case BOOTP_OPTION_DHCP_SERVER_IDENTIFIER:
              p++;
              READ_FORWARD_U32(dhcpClientInformation.serverAddress, p);
              break;

            case BOOTP_OPTION_END:
              p = message->options + sizeof (message->options);
              break;

            default: // ignoring unneeded options
              p += 1 + *p;
              break;
          }
        }

        if(dhcpMessageType == DHCP_MESSAGE_TYPE_OFFER)
        {
          send_request_message(offeredIp);

          state = STATE_WAITING_FOR_ACKNOWLEDGE;
          timeoutDuration = MINIMUM_TIMEOUT_DURATION;
          nabtoSetFutureStamp(&timer, timeoutDuration);
          NABTO_LOG_TRACE(("Offer received."));
        }
      }
      else if(nabtoIsStampPassed(&timer))
      {
        send_discover_message();

        if(timeoutDuration < MAXIMUM_TIMEOUT_DURATION)
        {
          timeoutDuration *= 2;
        }
        nabtoSetFutureStamp(&timer, timeoutDuration);

        NABTO_LOG_TRACE(("No offer received - trying again in %u seconds.", timeoutDuration / 1000));
      }
    }
      break;

    case STATE_WAITING_FOR_ACKNOWLEDGE:
    {
      uint32_t sourceIp;
      uint16_t sourcePort;
      uint16_t length = nabto_read(socket, nabtoCommunicationBuffer, sizeof (nabtoCommunicationBuffer), &sourceIp, &sourcePort);

      if(length > 0)
      {
        bool success = true;
        uint8_t dhcpMessageType;
        dhcp_message* message = (dhcp_message*) nabtoCommunicationBuffer;
        uint8_t* p = message->options;
        dhcp_client_information offer;

        memcpy(&offer, (const void*) &dhcpClientInformation, sizeof (offer));

        READ_U32(offer.localAddress, &message->yourIpAddress);

        while(success && p < (message->options + sizeof (message->options))) // go through entire options block
        {
          switch(*p++)
          {
            case BOOTP_OPTION_PADDING:
              break;

            case BOOTP_OPTION_DHCP_MESSAGE_TYPE:
              p++;
              dhcpMessageType = *p++;
              break;

            case BOOTP_OPTION_DHCP_SERVER_IDENTIFIER: // verify that we are talking to the correct DHCP server
            {
              uint32_t serverAddress;
              p++;
              READ_FORWARD_U32(serverAddress, p);
              if(serverAddress != dhcpClientInformation.serverAddress)
              {
                success = false;
              }
            }
              break;

            case BOOTP_OPTION_SUBNET_MASK:
              p++;
              READ_FORWARD_U32(offer.netmask, p);
              break;

            case BOOTP_OPTION_ROUTER:
              p++;
              READ_FORWARD_U32(offer.routerAddress, p);
              break;

            case BOOTP_OPTION_DOMAIN_NAME_SERVER:
              p++;
              READ_FORWARD_U32(offer.dnsAddress, p);
              break;

            case BOOTP_OPTION_IP_ADDRESS_LEASE_TIME:
              p++;
              READ_FORWARD_U32(offer.leaseTime, p);
              break;

            case BOOTP_OPTION_END:
              p = message->options + sizeof (message->options);
              break;

            default:
              p += 1 + *p;
              break;
          }
        }

        if(dhcpMessageType == DHCP_MESSAGE_TYPE_ACKNOWLEDGE && success)
        {
          state = STATE_BOUND;

          memcpy(&dhcpClientInformation, (const void*) &offer, sizeof (dhcpClientInformation));

          dhcpClientInformation.isBound = true;

          // indicates when lease renewal must be initiated
          timeoutDuration = dhcpClientInformation.leaseTime * 1000 / 2;
          nabtoSetFutureStamp(&timer, timeoutDuration);

          nabtoSetFutureStamp(&leaseExpirationTimer, dhcpClientInformation.leaseTime * 1000); // indicates when the lease has expired and state must be changed to unbound.

          NABTO_LOG_TRACE(("Acknowledge received."));
        }
      }
      else
      {
        if(dhcpClientInformation.isBound == false) // when the client is unbound...
        {
          if(nabtoIsStampPassed(&timer))
          {
            state = STATE_UNBOUND;
            timeoutDuration = MINIMUM_TIMEOUT_DURATION;

            NABTO_LOG_TRACE(("No acknowledge received - starting over."));
          }
        }
        else // when renewing...
        {
          if(nabtoIsStampPassed(&timer))
          {
            send_request_message(dhcpClientInformation.localAddress);

            if(timeoutDuration < MAXIMUM_TIMEOUT_DURATION)
            {
              timeoutDuration *= 2;
            }
            nabtoSetFutureStamp(&timer, timeoutDuration);

            NABTO_LOG_TRACE(("No acknowledge received - retrying renewal request."));
          }
          else if(nabtoIsStampPassed(&leaseExpirationTimer))
          {
            memset(&dhcpClientInformation, 0, sizeof (dhcpClientInformation));
            state = STATE_UNBOUND;
            NABTO_LOG_TRACE(("Lease expired!"));
          }
        }
      }
    }
      break;

    case STATE_BOUND:
      if(nabtoIsStampPassed(&timer)) // time to renew the lease?
      {
        send_request_message(dhcpClientInformation.localAddress);

        timeoutDuration = MINIMUM_TIMEOUT_DURATION;
        nabtoSetFutureStamp(&timer, timeoutDuration);
        state = STATE_WAITING_FOR_ACKNOWLEDGE;
        NABTO_LOG_TRACE(("Starting to renew lease."));
      }
      break;
  }
}

// Helpers

static void prepare_message(void)
{
  dhcp_message* message = (dhcp_message*) nabtoCommunicationBuffer;

  memset(message, 0, sizeof (dhcp_message));

  WRITE_U8(&message->opcode, BOOTP_MESSAGE_TYPE_BOOT_REQUEST);
  WRITE_U8(&message->hardwareType, BOOTP_HARDWARE_TYPE_ETHERNET);
  WRITE_U8(&message->hardwareAddressLength, 6);
  WRITE_U32(&message->transactionIdentifier, transactionIdentifier);

  if(dhcpClientInformation.isBound)
  {
    WRITE_U16(&message->flags, BOOTP_FLAG_UNICAST);
  }
  else
  {
    WRITE_U16(&message->flags, BOOTP_FLAG_BROADCAST);
  }

  WRITE_U32(&message->clientIpAddress, dhcpClientInformation.localAddress);
  WRITE_U8(&message->clientHardwareAddress[0], hardwareAddress[0]); // Hardware address (MSB)
  WRITE_U8(&message->clientHardwareAddress[1], hardwareAddress[1]); // Hardware address
  WRITE_U8(&message->clientHardwareAddress[2], hardwareAddress[2]); // Hardware address
  WRITE_U8(&message->clientHardwareAddress[3], hardwareAddress[3]); // Hardware address
  WRITE_U8(&message->clientHardwareAddress[4], hardwareAddress[4]); // Hardware address
  WRITE_U8(&message->clientHardwareAddress[5], hardwareAddress[5]); // Hardware address (LSB)
  WRITE_U32(&message->magicCookie, 0x63825363); // Magic cookie
}

static void send_message(void)
{
  if(dhcpClientInformation.isBound)
  {
    nabto_write(socket, nabtoCommunicationBuffer, sizeof (dhcp_message), dhcpClientInformation.serverAddress, 67);
  }
  else
  {
    nabto_write(socket, nabtoCommunicationBuffer, sizeof (dhcp_message), IP_BROADCAST_ADDRESS, 67);
  }
}

static void send_discover_message(void)
{
  dhcp_message* message = (dhcp_message*) nabtoCommunicationBuffer;
  uint8_t* options = message->options;

  prepare_message();

  // Add options

  // DHCP message type
  WRITE_FORWARD_U8(options, BOOTP_OPTION_DHCP_MESSAGE_TYPE);
  WRITE_FORWARD_U8(options, 1);
  WRITE_FORWARD_U8(options, DHCP_MESSAGE_TYPE_DISCOVER);

  // Parameter request list
  WRITE_FORWARD_U8(options, BOOTP_OPTION_PARAMETER_REQUEST_LIST);
  WRITE_FORWARD_U8(options, 3);
  WRITE_FORWARD_U8(options, PARAMETER_REQUEST_ITEM_SUBNET_MASK);
  WRITE_FORWARD_U8(options, PARAMETER_REQUEST_ITEM_ROUTER);
  WRITE_FORWARD_U8(options, PARAMETER_REQUEST_ITEM_DOMAIN_NAME_SERVER);

  // End
  WRITE_FORWARD_U8(options, BOOTP_OPTION_END);

  send_message();
}

static void send_request_message(uint32_t offeredIp)
{
  dhcp_message* message = (dhcp_message*) nabtoCommunicationBuffer;
  uint8_t* options = message->options;

  prepare_message();

  // Add options

  // DHCP message type
  WRITE_FORWARD_U8(options, BOOTP_OPTION_DHCP_MESSAGE_TYPE);
  WRITE_FORWARD_U8(options, 1);
  WRITE_FORWARD_U8(options, DHCP_MESSAGE_TYPE_REQUEST);

  // DHCP server identifier
  WRITE_FORWARD_U8(options, BOOTP_OPTION_DHCP_SERVER_IDENTIFIER);
  WRITE_FORWARD_U8(options, 4);
  WRITE_FORWARD_U32(options, dhcpClientInformation.serverAddress);

  // Parameter request list
  WRITE_FORWARD_U8(options, BOOTP_OPTION_PARAMETER_REQUEST_LIST);
  WRITE_FORWARD_U8(options, 3);
  WRITE_FORWARD_U8(options, PARAMETER_REQUEST_ITEM_SUBNET_MASK);
  WRITE_FORWARD_U8(options, PARAMETER_REQUEST_ITEM_ROUTER);
  WRITE_FORWARD_U8(options, PARAMETER_REQUEST_ITEM_DOMAIN_NAME_SERVER);

  // Rrequested IP address
  WRITE_FORWARD_U8(options, BOOTP_OPTION_REQUESTED_IP_ADDRESS);
  WRITE_FORWARD_U8(options, 4);
  WRITE_FORWARD_U32(options, offeredIp);

  // End
  WRITE_FORWARD_U8(options, BOOTP_OPTION_END);

  send_message();
}
