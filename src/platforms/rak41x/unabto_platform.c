/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_PLATFORM

#include <string.h>
#include <unabto/unabto_env_base.h>
#include <unabto/unabto_external_environment.h>
#include <rak_api.h>
#include <os.h>
#include <uart.h>

#define NUMBER_OF_SOCKETS               2
#define SEND_BUFFER_SIZE                1000

#define LED_LINK_PORT                   GPIOA
#define LED_LINK_PIN                    8

typedef struct
{
  bool isOpen;
  OS_SEM receiverSemaphore;
} internal_socket;

void SYS_Init(void);
bool poll_task_event_queue(void);

char* wifiSsid = NULL;
char* wifiKey = NULL;

static void wifi_callback(uint8_t value);

static void* mainTcb;

static volatile bool linkIsUp = false;
static uint32_t localAddress;
static uint32_t localMask;
static uint32_t gateway;
static uint32_t dnsServer;
static internal_socket sockets[NUMBER_OF_SOCKETS];
static char* sendBuffer;

OS_SEM loggingSemaphore;
  
// General

bool platform_initialize(void* tcb)
{
  OS_ERR osErr;
  
  mainTcb = tcb;
  
  SYS_Init();
  
  GPIO_SetBit(LED_LINK_PORT, LED_LINK_PIN);
  GPIO_Open(LED_LINK_PORT, GPIO_PMD_PMD8_OUTPUT, GPIO_PMD_PMD8_MASK);
  
#if NABTO_ENABLE_LOGGING
  uart_initialize(115200);
#endif
  
  // Initialize OS tick system
  OS_CPU_SysTickInit(SYS_GetHCLKFreq() / OS_CFG_TICK_RATE_HZ);
  
  OSSemCreate(&loggingSemaphore, NULL, 1, &osErr);
  if(osErr != OS_ERR_NONE)
  {
    NABTO_LOG_FATAL(("Unable to create logging semaphore"));
  }
  
  NABTO_LOG_INFO(("Initializing..."));
    
  if (RAK_DriverInit() != RAK_OK)
  {
    NABTO_LOG_FATAL(("Platform initialize failed!"));
  }

  {
    char mac[6];
    if (RAK_GetMacAddr(mac) != RAK_OK)
    {
      NABTO_LOG_FATAL(("RAK_GetMacAddr() failed!"));
    }
    NABTO_LOG_INFO(("MAC: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]));
  }
  
  {
    RAK_CONNECT param;
    param.mode = NET_MODE_STA;
    param.sec_mode = PSK_MODE_SEC;
    param.ssid = wifiSsid;
    param.psk = wifiKey;
    param.conn_handle = wifi_callback;
    if (RAK_ConnectAP(&param) != RAK_OK)
    {
      NABTO_LOG_FATAL(("Wifi connect error!"));
    }
  }

  while(linkIsUp == false); // wait for callback to set connection status

  {
    RAK_IPCONFIG dhcp;
    
    if (RAK_IPConfigDHCP(&dhcp) != RAK_OK)
    {
      NABTO_LOG_FATAL(("DHCP error!"));
    }
    
    localAddress = dhcp.addr;
    localMask = dhcp.mask;
    gateway = dhcp.gw;
    dnsServer = dhcp.dnsrv1;
    NABTO_LOG_TRACE(("DHCP: Address=" PRI_IP " mask=" PRI_IP " gateway=" PRI_IP " DNS=" PRI_IP, PRI_IP_FORMAT(localAddress), PRI_IP_FORMAT(localMask), PRI_IP_FORMAT(gateway), PRI_IP_FORMAT(dnsServer)));
  }
  
  memset(sockets, 0, sizeof(sockets));
  
  sendBuffer = RAK_SendMalloc(SEND_BUFFER_SIZE);
  
  return true;
}

void UART0_IRQHandler(void);
void platform_tick(void)
{ 
  UART0_IRQHandler();
}

// UDP

bool nabto_init_socket(uint32_t localAddr, uint16_t* localPort, nabto_socket_t* socket)
{
  uint8_t i;
  OS_ERR osErr;
  
  NABTO_NOT_USED(localAddr);
  
  for(i = 0; i < lengthof(sockets); i++)
  {
    if(sockets[i].isOpen == false)
    {
      uint16_t port = *localPort;
      if(port == 0)
      {
        port = 30000 + i;
      }
      
      if(RAK_UDPServer(mainTcb, port, i) == RAK_OK)
      {
        sockets[i].isOpen = true;
        
        OSSemCreate(&sockets[i].receiverSemaphore, NULL, 0, &osErr);
        if(osErr != OS_ERR_NONE)
        {
          NABTO_LOG_FATAL(("Unable to create receiver semaphore %u: %i", i, osErr));
        }

        *localPort = port;
        *socket = (nabto_socket_t)i;
        
        NABTO_LOG_TRACE(("nabto_init_socket %u: port=%u", *socket, *localPort));
        
        return true;
      }
    }
  }
  
  return false;
}

void nabto_close_socket(nabto_socket_t* socket)
{
  NABTO_LOG_FATAL(("Socket shutdown not supported!"));
  //printf("RAK_ShutDown(%u)=%u", *socket, RAK_ShutDown(*socket));
  //sockets[*socket].inUse = false;
}

ssize_t nabto_read(nabto_socket_t socket, uint8_t* buffer, size_t length, uint32_t* address, uint16_t* port)
{
  OS_ERR osErr;
  uint8_t* receiveBuffer;
  uint16_t receiveBufferLength;
  RAK_SOCKET_ADDR socketInfo;
 
  if(socket > NUMBER_OF_SOCKETS)
  {
    NABTO_LOG_FATAL(("Read on invalid socket!"));
    return 0;
  }
  
  if(sockets[socket].isOpen == false)
  {
    NABTO_LOG_ERROR(("Read on closed socket!"));
    return 0;
  }
  
  while(poll_task_event_queue());
  
  OSSemPend(&sockets[socket].receiverSemaphore, 0, OS_OPT_PEND_NON_BLOCKING, NULL, &osErr);
  if(osErr != OS_ERR_NONE)
  {
    return 0;
  }
  
  NABTO_LOG_TRACE(("nabto_read=%u receiving...", socket));
  
  if (RAK_RecvData(socket, &receiveBuffer, &receiveBufferLength) != RAK_OK)
  {
    return 0;
  }
  
  if(receiveBufferLength > length)
  {
    NABTO_LOG_TRACE(("nabto_read=%u oversize frame", socket));
    RAK_RecvFree(buffer);
    return 0;
  }
  
  memcpy(buffer, receiveBuffer, receiveBufferLength);
  
  RAK_RecvFree(receiveBuffer);

  RAK_GetSocketInfo(socket, &socketInfo);

  *address = socketInfo.dest_addr;
  *port = socketInfo.dest_port;
  
  NABTO_LOG_TRACE(("Received UDP packet from " PRI_IP ":%u length=%u", PRI_IP_FORMAT(*address), *port, receiveBufferLength));
  
  return receiveBufferLength;
}

ssize_t nabto_write(nabto_socket_t socket, const uint8_t* buffer, size_t length, uint32_t address, uint16_t port)
{
  int32_t result;
  
  if(length > SEND_BUFFER_SIZE)
  {
    return 0;
  }
  
  memcpy(sendBuffer, buffer, length);
  
  result = RAK_SendData(socket, sendBuffer, length, address, port);
  
  if(result != RAK_OK)
  {
    NABTO_LOG_ERROR(("RAK_SendData failed: %i", result));
    return 0;
  }
  
  NABTO_LOG_TRACE(("Sent UDP packet to " PRI_IP ":%u length=%u", PRI_IP_FORMAT(address), port, length));
  
  return length;
}

// DNS

void nabto_dns_resolver(void)
{ }

void nabto_dns_resolve(const char* host)
{ }

nabto_dns_status_t nabto_dns_is_resolved(const char* host, uint32_t* address)
{
  uint32_t result = RAK_DNSReq((char*)host, dnsServer);
  
  if(result == 0)
  {
    return NABTO_DNS_NOT_FINISHED;
  }
  
  *address = result;
  
  return NABTO_DNS_OK;
}

// Random

void nabto_random(uint8_t* buffer, size_t length)
{
  while(length--)
  {
    *buffer++ = length;
  }
}

// Timing

#define MAX_STAMP_DIFF 0x7fffffff;

nabto_stamp_t nabtoGetStamp(void)
{
  OS_ERR osErr;
  return OSTimeGet(&osErr);
}

bool nabtoIsStampPassed(nabto_stamp_t* stamp)
{
  return *stamp - nabtoGetStamp() > MAX_STAMP_DIFF;
}

nabto_stamp_diff_t nabtoStampDiff(nabto_stamp_t* newest, nabto_stamp_t* oldest)
{
  return (*newest - *oldest);
}

int nabtoStampDiff2ms(nabto_stamp_diff_t difference)
{
  return (int) difference;
}

// Helpers

static void wifi_callback(uint8_t value)
{
  NABTO_LOG_INFO(("WIFI callback: value = 0x%x", value));
    
  switch(value)
  {
    case CONN_STATUS_LINKED:
      linkIsUp = true;
      GPIO_ClrBit(LED_LINK_PORT, LED_LINK_PIN);
      break;
    case CONN_STATUS_DIS:
      linkIsUp = false;
      GPIO_SetBit(LED_LINK_PORT, LED_LINK_PIN);
      break;
  }
}

bool poll_task_event_queue(void)
{
  OS_ERR osErr;
  OS_MSG_SIZE size;
  uint32_t event;
 
  event = (uint32_t)OSTaskQPend(0, OS_OPT_PEND_NON_BLOCKING, &size, NULL, &osErr);
  
  if(osErr == OS_ERR_PEND_WOULD_BLOCK)
  {
    return false;
  }
  
  if(osErr != OS_ERR_NONE)
  {
    NABTO_LOG_ERROR(("Error in receiver task: %i", osErr));
    return false;
  }
  
  NABTO_LOG_TRACE(("Received event: %u", event));
  
  if (event & EVENT_RECV_DATA)
  {
    uint8_t index = event & EVENT_MASK;
    
    if(sockets[index].isOpen)
    {
      OSSemPost(&sockets[index].receiverSemaphore, OS_OPT_POST_NO_SCHED, &osErr);
      if(osErr != OS_ERR_NONE)
      {
        NABTO_LOG_ERROR(("Unable to post on socket receive semaphore!"));
      }
      
      NABTO_LOG_TRACE(("Received packet on socket %u", index));
    }
    else
    {
      NABTO_LOG_TRACE(("Data on non-open socket."));
    }
  }
  else
  {
    NABTO_LOG_TRACE(("Dropped unexpected event."));
  }
  
  return true;
}

#if NABTO_ENABLE_LOGGING

const char* clean_filename(const char* path)
{
  const char* filename = path + strlen(path);

  while(filename > path)
  {
    if(*filename == '/' || *filename == '\\')
    {
      return filename + 1;
    }
    filename--;
  }

  return path;
}

#endif
