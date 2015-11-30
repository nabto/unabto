#ifndef _UNABTO_DNS_FALLBACK_SESSION_H_
#define _UNABTO_DNS_FALLBACK_SESSION_H_

#include <unabto_platform_types.h>
#include "unabto_dns_fallback_protocol.h"

void unabto_dns_fallback_session_send_open_request(unabto_dns_fallback_session* session);

void unabto_dns_fallback_session_open_response(unabto_dns_fallback_session* session, unabto_dns_fallback_response_header* response, uint8_t* buffer, uint8_t* end);

#endif
