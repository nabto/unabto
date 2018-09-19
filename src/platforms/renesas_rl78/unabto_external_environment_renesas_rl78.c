/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
/**
 * @file
 * The external environment for uNabto Renesas RL78 - Implementation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <CmdLib/AtCmdLib.h>
#include <system/mstimer.h>
#include <CmdLib/GainSpan_SPI.h>
#include "Apps.h"
#include "led.h"

#include "unabto/unabto_common_main.h"
#include "unabto/unabto_env_base.h"
#include "unabto/unabto_external_environment.h"

/** 
 * Fill buffer with random content.
 * @param buf  the buffer
 * @param len  the length of the buffer
 */
void nabto_random(uint8_t* buf, size_t len)
{
  int i;
  if (NULL == buf) {
    return;
  }
  for (i=0; i < len; i++) {
    *buf++ = rand();
  }
}

/**
 * Initialise a udp socket.  This function is called for every socket
 * uNabto creates, this will normally occur two times. One for local
 * connections and one for remote connections.
 *
 * @param localPort    The local port to bind to.
 *                     A port number of 0 gives a random port.
 * @param socket       To return the created socket descriptor.
 * @return             true iff successfull
 */
bool nabto_init_socket(uint16_t* localPort, nabto_socket_t* socket)
{
  uint8_t cid = 0;
  
  // Remote connection calls with localPort 0
  if (*localPort == 0) {
    *localPort = 5123;    // should be random
  }
  
  if (AtLibGs_UDPServer_Start(*localPort, &cid) != ATLIBGS_MSG_ID_OK) {
    NABTO_LOG_TRACE(("failed to create socket"));
    return false;
  }

  if (cid == ATLIBGS_INVALID_CID) {
    NABTO_LOG_TRACE(("invalid CID"));
    return false;
  }
  
  *socket = cid;

  return true;
}

/**
 * Close a socket.
 * Close can be called on already closed sockets. And should tolerate this behavior.
 *
 * @param socket the socket to be closed
 */
void nabto_close_socket(nabto_socket_t* socket)
{
  if (socket == NULL || *socket == NABTO_INVALID_SOCKET) {
    return;
  }

  if (AtLibGs_Close(*socket) != ATLIBGS_MSG_ID_OK) {
    return;
  }
  *socket = NABTO_INVALID_SOCKET;
}

/**
 * Read message from network (non-blocking).
 * Memory management is handled by the callee.
 *
 * @param socket  the UDP socket
 * @param buf     destination of the received bytes
 * @param len     length of destination buffer
 * @param addr    the senders IP address (host byte order)
 * @param port    the senders UDP port (host byte order)
 * @return        the number of bytes received
 */
ssize_t nabto_read(nabto_socket_t socket,
                   uint8_t*       buf,
                   size_t         len,
                   struct nabto_ip_address*      addr,
                   uint16_t*      port)
{
  ATLIBGS_UDPMessage at_msg;
  
  if (AtLibGs_WaitForUDPMessage(10) == ATLIBGS_MSG_ID_OK) {
    uint8_t temp_port[2];
    int temp_ip[4];
    
    AtLibGs_ParseUDPData(G_received, G_receivedCount, &at_msg);
    
    memcpy(buf, at_msg.message, at_msg.numBytes);
    
    // Convert IP
    sscanf(at_msg.ip, _F8_ "." _F8_ "." _F8_ "." _F8_, &temp_ip[0], &temp_ip[1], &temp_ip[2], &temp_ip[3]);
    uint32_t ip = (uint32_t)temp_ip[0] << 24 |
                  (uint32_t)temp_ip[1] << 16 |
                  (uint32_t)temp_ip[2] << 8  |
                  (uint32_t)temp_ip[3];

    addr->type = NABTO_IP_V4;
    addr->addr.ipv4 = ip;
    
    // Convert port, host order
    temp_port[0] = (uint8_t)(at_msg.port >> 8);
    temp_port[1] = (uint8_t)(at_msg.port);
    *port = (uint16_t)temp_port[0] << 8 | temp_port[1];

    len = at_msg.numBytes;
    
    // Clear the RX buffer for the next reception
    App_PrepareIncomingData();
    
    return len;
  }
  App_PrepareIncomingData();
  return 0;
}

/**
 * Write message to network (blocking) The memory allocation and
 * deallocation for the buffer is handled by the caller.
 *
 * @param socket  the UDP socket
 * @param buf   the bytes to be sent
 * @param len   number of bytes to be sent
 * @param addr  the receivers IP address (host byte order)
 * @param port  the receivers UDP port (host byte order)
 * @return      true when success
 */
ssize_t nabto_write(nabto_socket_t socket,
                    const uint8_t* buf,
                    size_t         len,
                    struct nabto_ip_address       addr,
                    uint16_t       port)
{
  char ip[4];
  char s_ip[17];
  if (addr->type != NABTO_IP_V4) {
      return 0;
  }
  
  ip[3] = (uint8_t)addr->addr.ipv4;
  ip[2] = (uint8_t)(addr->addr.ipv4 >> 8);
  ip[1] = (uint8_t)(addr->addr.ipv4 >> 16);
  ip[0] = (uint8_t)(addr->addr.ipv4 >> 24);
  sprintf(s_ip, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  
  AtLibGs_BulkDataTrans(true);

  if (AtLibGs_BulkUDP(socket, buf, len, s_ip, port) == ATLIBGS_MSG_ID_OK) {
    AtLibGs_BulkDataTrans(false);
    return true;
  }

  AtLibGs_BulkDataTrans(false);
  return false;
}

/**
 * Get Current time stamp
 * @return current time stamp
 */
nabto_stamp_t nabtoGetStamp(void)
{
  return MSTimerGet();
}

bool nabtoIsStampPassed(nabto_stamp_t *stamp) {
  return (nabtoGetStamp() - *stamp) > 0;
}

nabto_stamp_diff_t nabtoStampDiff(nabto_stamp_t * newest, nabto_stamp_t * oldest) {
  return (*newest - *oldest);
}

int nabtoStampDiff2ms(nabto_stamp_diff_t diff) {
  return (int) diff;
}

void nabto_resolve_ipv4(uint32_t ipv4, struct nabto_ip_address* ip) {
    ip->type = NABTO_IP_V4;
    ip->addr.ipv4 = ipv4;
}

/**
 * start resolving an ip address
 * afterwards nabto_resolve_dns will be called until the address is resolved
 */
void nabto_dns_resolve(const char* id)
{
  if (id != NULL) {
    AtLibGs_DNSLookup((char*)id);
  }
}

/**
 * resolve an ipv4 dns address
 * if resolving fails in first attempt we call the function later to 
 * see if the address is resolved. The id is always constant for a device
 * meaning the address could be hardcoded but then devices will fail if 
 * the basestation gets a new ip address.
 * @param id      name controller hostname
 * @param v4addr  pointer to ipaddress
 * @return false if address is not resolved yet
 */
nabto_dns_status_t nabto_dns_is_resolved(const char *id, uint32_t* v4addr)
{
/* To use or not to use DNS response, that is the question */
#if 1
  uint32_t c_ip = 0xC3F99F9F;  // static remote ip: 195.249.159.159
  *v4addr = c_ip;
  return NABTO_DNS_OK;
#else
  char s_ip[17];
  
  if (AtLibGs_ParseDNSLookupResponse(s_ip)) {
    // Host order little endian
    int temp_ip[4];
    sscanf(s_ip, _F8_ "." _F8_ "." _F8_ "." _F8_, &temp_ip[0], &temp_ip[1], &temp_ip[2], &temp_ip[3]);
    uint32_t ip = (uint32_t)temp_ip[0] << 24 |
                  (uint32_t)temp_ip[1] << 16 |
                  (uint32_t)temp_ip[2] << 8  |
                  (uint32_t)temp_ip[3];
    *v4addr = ip;
    return NABTO_DNS_OK;
  }
  return NABTO_DNS_NOT_FINISHED;
#endif
}
