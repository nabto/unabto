/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_NETWORK

#include <unabto/unabto_env_base.h>
#include <TCPIP Stack/TCPIP.h>
#include <unabto/unabto_external_environment.h>

enum
{
  STATE_START_RESOLVE,
  STATE_RESOLVING,
  STATE_SUCCESS,
  STATE_ERROR
};

static uint8_t state = STATE_SUCCESS;
static uint32_t address = 0;
static const char* devId = NULL;

void nabto_dns_resolver(void)
{
  switch(state)
  {
    case STATE_START_RESOLVE:
      if(devId != NULL)
      {
        DNSEndUsage();
        if(DNSBeginUsage())
        {
          NABTO_LOG_INFO(("Resolving"));

          DNSResolve((BYTE*) devId, DNS_TYPE_A);

          state = STATE_RESOLVING;
        }
      }
      break;

    case STATE_RESOLVING:
      if(DNSIsResolved((DWORD_VAL*) & address))
      {
        DNSEndUsage();

        if(address == 0)
        {
          state = STATE_ERROR;
        }
        else
        {
          address = swapl(address);
          NABTO_LOG_INFO(("DNS success: " PRIip, MAKE_IP_PRINTABLE(address)));

          state = STATE_SUCCESS;
        }
      }
      break;

    case STATE_SUCCESS:
      break;

    case STATE_ERROR:
      break;
  }
}

void nabto_resolve_ipv4(uint32_t ipv4, struct nabto_ip_address* ip) {
    ip->type = NABTO_IP_V4;
    ip->addr.ipv4 = ipv4;
}

void nabto_dns_resolve(const char* id)
{
  devId = id;
  state = STATE_START_RESOLVE;
}

nabto_dns_status_t nabto_dns_is_resolved(const char* id, uint32_t* addr)
{
  if(id != devId)
  {
    NABTO_LOG_INFO(("DNS: Bad id!"));
    return NABTO_DNS_ERROR; // only one
  }
  else if(state == STATE_SUCCESS)
  {
    NABTO_LOG_INFO(("DNS: Success"));
    *addr = address;
    return NABTO_DNS_OK;
  }
  else if(state == STATE_ERROR)
  {
    NABTO_LOG_INFO(("DNS: Error!"));
    return NABTO_DNS_ERROR;
  }
  else
  {
    NABTO_LOG_INFO(("DNS: Not finished"));
    return NABTO_DNS_NOT_FINISHED;
  }
}
