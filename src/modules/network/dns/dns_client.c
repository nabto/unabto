/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_NETWORK

#include "dns_client.h"
#include <unabto/unabto_memory.h>
#include <unabto/unabto_util.h>
#include <string.h>

#define TIMEOUT                                                     500
#define MAXIMUM_ATTEMPTS                                            10

#define FLAG_RESPONSE                                               0x8000
#define FLAG_OPCODE_MASK                                            0x7800
#define FLAG_OPCODE_STANDARD_QUERY                                  0x0000
#define FLAG_OPCODE_INVERSE_QUERY                                   0x0800
#define FLAG_OPCODE_STATUS                                          0x1000
#define FLAG_OPCODE_NOTIFY                                          0x2000
#define FLAG_OPCODE_UPDATE                                          0x2800
#define FLAG_AUTHORITATIVE_ANSWER                                   0x0400
#define FLAG_TRUNCATED                                              0x0200
#define FLAG_RECURSION_DESIRED                                      0x0100
#define FLAG_RECURSION_AVAILABLE                                    0x0080
#define FLAG_ANSWER_AUTHENTICATED                                   0x0020
#define FLAG_NON_AUTHENTICATED_DATA                                 0x0010
#define FLAG_RESPONSE_CODE_MASK                                     0x000f
#define FLAG_RESPONSE_CODE_NO_ERROR                                 0x0000
#define FLAG_RESPONSE_CODE_FORMAT_ERROR                             0x0001
#define FLAG_RESPONSE_CODE_SERVER_FAILURE                           0x0002
#define FLAG_RESPONSE_CODE_NAME_ERROR                               0x0003
#define FLAG_RESPONSE_CODE_NOT_IMPLEMENTED                          0x0004
#define FLAG_RESPONSE_CODE_REFUSED                                  0x0005
#define FLAG_RESPONSE_CODE_YX_DOMAIN                                0x0006
#define FLAG_RESPONSE_CODE_YR_RR_SET                                0x0007
#define FLAG_RESPONSE_CODE_NX_RR_SET                                0x0008
#define FLAG_RESPONSE_CODE_NOT_AUTHORITATIVE                        0x0009
#define FLAG_RESPONSE_CODE_NOT_ZONE                                 0x000a

#define TYPE_A                                                      0x0001

#define CLASS_IN                                                    0x0001

typedef struct
{
  uint16_t transactionIdentifier;
  uint16_t flags;
  uint16_t questionCount;
  uint16_t answerRecordCount;
  uint16_t authorityRecordCount;
  uint16_t additionalRecordCount;
  uint8_t data[];
} dns_packet;

enum
{
  STATE_UNINITIALIZED,
  STATE_IDLE,
  STATE_OPEN_SOCKET,
  STATE_RESOLVE,
  STATE_WAITING_FOR_RESPONSE,
  STATE_RESOLVED,
  STATE_NOT_FOUND,
  STATE_NO_SOCKET
};

static uint8_t* dump_name(uint8_t* p);

static uint32_t serverIp;
static nabto_socket_t clientSocket;
static uint8_t state = STATE_UNINITIALIZED;
static nabto_stamp_t timer;
static uint16_t transactionIdentifier;

static const char* currentHost;
static uint8_t currentAttempts;
static uint32_t currentIp;

void dns_client_initialize(uint32_t dnsServerIp)
{
  serverIp = dnsServerIp;
  state = STATE_IDLE;

  NABTO_LOG_TRACE(("Initialized: server=" PRIip, MAKE_IP_PRINTABLE(dnsServerIp)));
}

void dns_client_tick(void)
{
  switch(state)
  {
    case STATE_OPEN_SOCKET:
    {
      uint16_t localPort = 0;
      if(nabto_init_socket(&localPort, &clientSocket))
      {
        state = STATE_RESOLVE;
        NABTO_LOG_TRACE(("Socket opened."));
      }
      else
      {
        state = STATE_NO_SOCKET;
        NABTO_LOG_TRACE(("Unable to open socket."));
      }
    }
      break;

    case STATE_RESOLVE:
    {
      dns_packet* packet = (dns_packet*) nabtoCommunicationBuffer;
      uint8_t* p = packet->data;
      uint8_t* name = (uint8_t*) currentHost;
      uint8_t* labelLengthPointer;

      memset(nabtoCommunicationBuffer, 0, nabtoCommunicationBufferSize);
      WRITE_U16(&packet->transactionIdentifier, transactionIdentifier);
      WRITE_U16(&packet->flags, FLAG_OPCODE_STANDARD_QUERY | FLAG_RECURSION_DESIRED);
      WRITE_U16(&packet->questionCount, 1);

      while(*name != 0)
      {
        labelLengthPointer = p++;
        *labelLengthPointer = 0;
        while(*name != '.' && *name != 0)
        {
          WRITE_FORWARD_U8(p, *name++);
          *labelLengthPointer = *labelLengthPointer + 1;
        }

        if(*name == '.')
        {
          name++;
        }
      }
      WRITE_FORWARD_U8(p, 0); // terminating zero

      WRITE_FORWARD_U16(p, TYPE_A);
      WRITE_FORWARD_U16(p, CLASS_IN);

      nabto_write(clientSocket, nabtoCommunicationBuffer, p - nabtoCommunicationBuffer, serverIp, 53);

      state = STATE_WAITING_FOR_RESPONSE;
      nabtoSetFutureStamp(&timer, TIMEOUT);

      NABTO_LOG_TRACE(("Sent request."));
    }
      break;

    case STATE_WAITING_FOR_RESPONSE:
    {
      struct nabto_ip_address sourceIp;
      uint16_t sourcePort;
      dns_packet* dnsPacket = (dns_packet*) nabtoCommunicationBuffer;
      uint16_t length = (uint16_t) nabto_read(clientSocket, nabtoCommunicationBuffer, sizeof (nabtoCommunicationBuffer), &sourceIp, &sourcePort);
      uint16_t type;
      uint16_t classCode;
      uint32_t timeToLive;
      uint16_t dataLength;

      if(length > 0)
      {
        uint8_t* p = dnsPacket->data;
        bool done = false;

        //        NABTO_LOG_BUFFER(NABTO_LOG_SEVERITY_TRACE, ("DNS response:"), buffer, length);

        READ_U16(dnsPacket->transactionIdentifier, &dnsPacket->transactionIdentifier);
        READ_U16(dnsPacket->flags, &dnsPacket->flags);
        READ_U16(dnsPacket->questionCount, &dnsPacket->questionCount);
        READ_U16(dnsPacket->answerRecordCount, &dnsPacket->answerRecordCount);
        READ_U16(dnsPacket->authorityRecordCount, &dnsPacket->authorityRecordCount);
        READ_U16(dnsPacket->additionalRecordCount, &dnsPacket->additionalRecordCount);

        if(dnsPacket->transactionIdentifier == transactionIdentifier && (dnsPacket->flags & (FLAG_RESPONSE | FLAG_OPCODE_MASK | FLAG_RESPONSE_CODE_MASK)) == (FLAG_RESPONSE | FLAG_OPCODE_STANDARD_QUERY | FLAG_RESPONSE_CODE_NO_ERROR))
        {
          // dump queries
          while(dnsPacket->questionCount--)
          {
            p = dump_name(p);
            p += 2 + 2;
          }

          // parse answers
          while(done == false && dnsPacket->answerRecordCount--)
          {
            p = dump_name(p);
            READ_FORWARD_U16(type, p);
            READ_FORWARD_U16(classCode, p);
            READ_FORWARD_U32(timeToLive, p);
            READ_FORWARD_U16(dataLength, p);

            if(type == TYPE_A && classCode == CLASS_IN && dataLength == 4)
            {
              READ_U32(currentIp, p);
              //              NABTO_LOG_TRACE(("IP found at offset %u: " PRIip, p - buffer, MAKE_IP_PRINTABLE_(currentIp)));
              done = true;
            }

            p += dataLength;
          }

          if(done)
          {
            state = STATE_RESOLVED;
            NABTO_LOG_TRACE(("Host resolved."));
          }
        }
      }
      else if(nabtoIsStampPassed(&timer))
      {
        if(++currentAttempts >= MAXIMUM_ATTEMPTS)
        {
          state = STATE_NOT_FOUND;
          NABTO_LOG_TRACE(("Unable to resolve host."));
        }
        else
        {
          state = STATE_RESOLVE;
        }
      }
    }
      break;
  }
}

void dns_client_nabto_dns_resolve(const char* host)
{
  if(state != STATE_IDLE)
  {
//    return NABTO_DNS_ERROR;
  }
  else
  {
    currentHost = host;
    currentAttempts = 0;
    transactionIdentifier++;
    state = STATE_OPEN_SOCKET;

//    return NABTO_DNS_OK;
  }
}

nabto_dns_status_t dns_client_nabto_dns_is_resolved(const char* host, uint32_t* ip)
{
  nabto_dns_status_t status;

  // is the specified host being resolved?
  if(host != currentHost)
  {
    return NABTO_DNS_ERROR;
  }

  switch(state)
  {
    case STATE_OPEN_SOCKET:
    case STATE_RESOLVE:
    case STATE_WAITING_FOR_RESPONSE:
      return NABTO_DNS_NOT_FINISHED;

    case STATE_RESOLVED:
      *ip = currentIp;
      status = NABTO_DNS_OK;
      break;

    case STATE_NOT_FOUND:
      status = NABTO_DNS_ERROR;
      break;

    case STATE_NO_SOCKET:
      status = NABTO_DNS_ERROR;
      break;

    default:
      return NABTO_DNS_ERROR;
  }

  nabto_close_socket(&clientSocket);
  state = STATE_IDLE;

  return status;
}

// Helpers

static uint8_t* dump_name(uint8_t* p)
{
  while(*p)
  {
    if(*p >= 0xc0)
    {
      return p + 2;
    }
    else
    {
      p += 1 + *p;
    }
  }

  p++;

  return p;
}
