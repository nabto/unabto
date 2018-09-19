/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
/**
 * @file
 * The external environment for uNabto RTX4100 - Implementation.
 */

#include "unabto/unabto_env_base.h"
#include "Protothreads/Protothreads.h"
#include "unabto/unabto_util.h"
#include "unabto/unabto_common_main.h"
#include "unabto/unabto_external_environment.h"

extern RsListEntryType PtList; // MUST be declared by main application

//#define NABTO_LOG_TRACE(msg) rtx4100_shell_log(msg);

typedef struct
{
  uint32_t remote_addr;
  uint16_t remote_port;
  char* bufferPtr;
  size_t bufferLength;
  nabto_socket_t handle;
  bool pending;
} rx_buffer_t;

rx_buffer_t rx_buffer;

uint16_t next_port = 11111;

typedef struct
{
  struct pt child_pt;
} create_sockets_data_type;

typedef struct
{
  nabto_socket_t* socket;
  uint32_t addr;
  uint16_t port;
  bool pending;
} socket_request_t;

nabto_dns_status_t dns_status;
uint32_t dns_addr;

#define MAX_SOCKET_REQUESTS 2
socket_request_t sr[MAX_SOCKET_REQUESTS];

static int activePacketBuffers = 0;

PT_THREAD(free_buffers(struct pt * Pt, const RosMailType* Mail)) {
  PT_BEGIN(Pt);

  while(true)
  {
    PT_YIELD_UNTIL(Pt, IS_RECEIVED(API_SOCKET_SEND_CFM));// && ((ApiSocketSendCfmType*)Mail)->Handle == sr[0].socket);
    //if(IS_RECEIVED(API_SOCKET_SEND_CFM))
    {
      NABTO_LOG_TRACE(("Send to confirmation received"));
      ApiSocketSendCfmType* send_cfm = (ApiSocketSendCfmType*)Mail;
      RcHeapFree(send_cfm->BufferPtr);
      activePacketBuffers--;
    }
  }
  PT_END(Pt);
}

PT_THREAD(create_socket(struct pt * Pt, const RosMailType* Mail)) {
  PT_BEGIN(Pt);

  SendApiSocketCreateReq(COLA_TASK, ASD_AF_INET, AST_DGRAM, 0, (rsuint32)sr);
  PT_WAIT_UNTIL(Pt, IS_RECEIVED(API_SOCKET_CREATE_CFM) && ((ApiSocketCreateCfmType*)Mail)->Instance == (rsuint32)sr);

  if(((ApiSocketCreateCfmType*)Mail)->Status == RSS_SUCCESS)
  {
    // Socket created
    *(sr[0].socket) = ((ApiSocketCreateCfmType*)Mail)->Handle;
  }
  else
  {
    NABTO_LOG_TRACE(("Failed to create socket"));
    PT_EXIT(Pt);
  }

  ApiSocketAddrType bindTo;
  bindTo.Domain = ASD_AF_INET;
  bindTo.Port = sr[0].port;
  bindTo.Ip.V4.Addr = sr[0].addr;
  SendApiSocketBindReq(COLA_TASK, *(sr[0].socket), bindTo);
  PT_WAIT_UNTIL(Pt, IS_RECEIVED(API_SOCKET_BIND_CFM));

  if(((ApiSocketBindCfmType*)Mail)->Status != RSS_SUCCESS)
  {
    NABTO_LOG_TRACE(("Failed to bind socket"));
    PT_EXIT(Pt);
  }
  NABTO_LOG_TRACE(("Socket created all ok"));

  PT_END(Pt);
}

PtEntryType* create_sockets_thread;
PT_THREAD(create_sockets(struct pt * Pt, const RosMailType* Mail)) {
  create_sockets_data_type *pInst = (create_sockets_data_type*)PtInstDataPtr;
  PT_BEGIN(Pt);
  while(1)
  {
    PT_YIELD_UNTIL(Pt, sr[0].pending);
    PT_SPAWN(Pt, &pInst->child_pt, create_socket(&pInst->child_pt, Mail));
    int i;
    for(i = 0; i < MAX_SOCKET_REQUESTS-1; i++)
    {
      sr[i] = sr[i+1];
      sr[MAX_SOCKET_REQUESTS-1].pending = false;
    }
  }
  PT_END(Pt);
}

PT_THREAD(PtOnData(struct pt *Pt, const RosMailType* Mail)) {  
  PT_BEGIN(Pt);
    
  while(1)
  {
    PT_YIELD_UNTIL(Pt, (IS_RECEIVED(API_SOCKET_RECEIVE_FROM_IND) ||
                        IS_RECEIVED(API_DNS_CLIENT_RESOLVE_CFM) ||
                        IS_RECEIVED(API_DNS_CLIENT_RESOLVE_CNAME_CFM) ||
                        IS_RECEIVED(API_DNS_CLIENT_ADD_SERVER_CFM)
                       ) && !PtMailHandled);
    PtMailHandled = TRUE;
    
    NABTO_LOG_TRACE(("Nabto message"));
    
    // Received DNS add status
    if(IS_RECEIVED(API_DNS_CLIENT_ADD_SERVER_CFM))
    {
      ApiDnsClientAddServerCfmType *cfm = (ApiDnsClientAddServerCfmType*)Mail;
      if(cfm->Status == RSS_SUCCESS)
      {
        NABTO_LOG_TRACE(("Successfully added a dns server"));
      }
      else
      {
        NABTO_LOG_TRACE(("Failed to add a dns server"));
      }
    }
    
    // Received DNS resolve
    if(IS_RECEIVED(API_DNS_CLIENT_RESOLVE_CFM))
    {
      ApiDnsClientResolveCfmType* cfm = (ApiDnsClientResolveCfmType*)Mail;

      if(cfm->Status != RSS_SUCCESS)
      {
        dns_status = NABTO_DNS_ERROR;
        NABTO_LOG_TRACE(("Failed to resolve dns"));
      }
      else
      {
        dns_status = NABTO_DNS_OK;
        dns_addr = cfm->IpV4;
        NABTO_LOG_TRACE(("Dns resolving succeeded"));
      }
    }
    
    // Received DNS CNAME resolve (not being used atm)
    if(IS_RECEIVED(API_DNS_CLIENT_RESOLVE_CNAME_CFM))
    {
      ApiDnsClientResolveCnameCfmType* cfm = (ApiDnsClientResolveCnameCfmType*)Mail;
      if(cfm->Status != RSS_SUCCESS)
      {
        dns_status = NABTO_DNS_ERROR;
        NABTO_LOG_TRACE(("Failed to resolve dns"));
      }
      else
      {
        //dns_status = NABTO_DNS_OK;
        //dns_addr = cfm-> IpV4;
        //NABTO_LOG_TRACE(("Dns resolving succeeded"));
        SendApiDnsClientResolveReq(COLA_TASK, false, cfm->AliasNameLength, (rsuint8*)cfm->AliasName);
      }
    }
    
    // Received nabto message
    if(IS_RECEIVED(API_SOCKET_RECEIVE_FROM_IND))
    {      
      ApiSocketReceiveFromIndType* m = (ApiSocketReceiveFromIndType*)Mail;
      
      // Dodge DHCP messages by looking at size (dangerous)
      if (m->BufferLength > 200)
      {
        NABTO_LOG_TRACE(("DHCP message dodged"));
      }
      else if (rx_buffer.pending)
      {
        NABTO_LOG_TRACE(("Received packet but rx buffer is full, discarding message"));
        SendApiSocketFreeBufferReq(COLA_TASK, m->Handle, m->BufferPtr);
      }
      else
      {
        rx_buffer.remote_addr = m->Addr.Ip.V4.Addr;
        rx_buffer.remote_port = m->Addr.Port;
        rx_buffer.bufferPtr = (char*)m->BufferPtr;
        rx_buffer.bufferLength = m->BufferLength;
        rx_buffer.handle = m->Handle;
        rx_buffer.pending = true;
        NABTO_LOG_TRACE(("Received a packet from %u:%u handle: %u", 
                      rx_buffer.remote_addr, rx_buffer.remote_port, 
                      rx_buffer.handle));

        unabto_tick();
      }
    }
  }
  PT_END(Pt);
}

socket_request_t* next_free_socket_request() {
  int i;
  for(i = 0; i < MAX_SOCKET_REQUESTS; i++)
  {
    if(!sr[i].pending)
    {
      return &sr[i];
    }
  }
  return NULL;
}


bool nabto_init_socket(uint16_t* localPort, nabto_socket_t* socket) {

  if(*localPort == 0)
  {
    *localPort = next_port++;
  }

  socket_request_t* n = next_free_socket_request();
  if(n == NULL)
  {
    NABTO_LOG_TRACE(("no more free socket requests"));
    return false;
  }

  n->socket = socket;
  n->port = *localPort;
  n->addr = INADDR_ANY;
  n->pending = true;
  *socket = NABTO_INVALID_SOCKET;
  return true;
}

void nabto_close_socket(nabto_socket_t* socket) {
  if(socket != NULL)
  {
    SendApiSocketCloseReq(COLA_TASK, *socket);
    *socket = NABTO_INVALID_SOCKET;
  }
} 

ssize_t nabto_read(nabto_socket_t socket, 
                   uint8_t* buf, size_t buflen,
                   struct nabto_ip_address* addr,
                   uint16_t* port) {
  if(socket != 0)
  {
    if(rx_buffer.pending && rx_buffer.handle == socket)
    {
      int length = RSMIN(buflen, rx_buffer.bufferLength);
      memcpy(buf, rx_buffer.bufferPtr, length);
      addr->type = NABTO_IP_V4;
      addr->addr.ipv4 = rx_buffer.remote_addr;
      *port = rx_buffer.remote_port;
      SendApiSocketFreeBufferReq(COLA_TASK, rx_buffer.handle, (rsuint8*)rx_buffer.bufferPtr);
      rx_buffer.pending = false;
      NABTO_LOG_TRACE(("Read packet, lenght %i",length));
      return length;
    }
  }
  return 0;
}

ssize_t nabto_write(nabto_socket_t socket,
                    const uint8_t* buf,
                    size_t len,
                    struct nabto_ip_address* addr,
                    uint16_t port) {
  if(addr->type != NABTO_IP_V4 || port == 0)
  {
    return 0;
  }
  if(socket == 0)
  {
    NABTO_LOG_TRACE(("Nabto write, socket is 0"));
    return 0;
  }

  if (activePacketBuffers > 5) {
    NABTO_LOG_ERROR(("Too many outstanding packets"));
    return 0;
  }
  
  uint8_t* fresh_buffer = (uint8_t*)RcHeapAllocEx(len, RHF_NULL_ON_FULL);
  if(fresh_buffer == NULL)
  {
    NABTO_LOG_ERROR(("No more heap space :("));
    return 0;
  }
  activePacketBuffers++;
  
  memcpy(fresh_buffer, buf, len);

  ApiSocketAddrType sendTo;
  sendTo.Domain = ASD_AF_INET;
  sendTo.Port = port;
  sendTo.Ip.V4.Addr = addr->addr.ipv4;
  SendApiSocketSendToReq(COLA_TASK, socket, fresh_buffer, len, 0, sendTo);
  NABTO_LOG_TRACE(("Send packet length: %i %u:%u handle: %u", len, addr->addr.ipv4, port, socket));
  return len;
}

bool unabto_environment_rtx4100_initialized = false;
bool platform_initialize() {
  if(!unabto_environment_rtx4100_initialized)
  {
    int i;
    rx_buffer.pending = false;
    for(i = 0; i < MAX_SOCKET_REQUESTS; i++)
    {
      sr[i].pending = false;
    }

    ApiIpV6AddressType ipv6;
    ApiIpV4AddressType ipv4 = 0x08080808;
    SendApiDnsClientAddServerReq(COLA_TASK, ipv4, ipv6);

    create_sockets_data_type* s = RcHeapAlloc(sizeof(create_sockets_data_type));
    PtStart(&PtList, create_sockets, NULL, s);
    PtStart(&PtList, PtOnData, NULL, NULL);

    unabto_environment_rtx4100_initialized = true;
    PtStart(&PtList, free_buffers, NULL, NULL);
  }

  return true;
}

void nabto_random(uint8_t* buf, size_t len) {

}

void nabto_dns_resolve(const char* id) {
  SendApiDnsClientResolveReq(COLA_TASK, false, strlen(id), (rsuint8*)id);
  //SendApiDnsClientResolveCnameReq(COLA_TASK, strlen(id), (rsuint8*)id);
  //NABTO_LOG_TRACE(("Resolve DNS"));
  dns_status = NABTO_DNS_NOT_FINISHED;
}

nabto_dns_status_t nabto_dns_is_resolved(const char* id, uint32_t* v4addr) {
  if(dns_status == NABTO_DNS_OK)
  {
    *v4addr = dns_addr;
    //NABTO_LOG_TRACE(("DNS resolved"));
  }
  return dns_status;
}

