#ifndef _UNABTO_DNS_FALLBACK_PROBE_H_
#define _UNABTO_DNS_FALLBACK_PROBE_H_

#include "unabto_dns_fallback_protocol.h"

enum {
    UDFP_TIMEOUT_TEST_MAX_RETRIES = 3,
    UDFP_TIMEOUT_TEST_MIN_TIMEOUT = 500,
    UDFP_TIMEOUT_TEST_MAX_TIMEOUT = 30000
};

void unabto_dns_fallback_probe_start_timeout_test(unabto_dns_fallback_session* session);

void unabto_dns_fallback_probe_event(unabto_dns_fallback_session* session);

void unabto_dns_fallback_probe_timeout_test_response(unabto_dns_fallback_session* session, unabto_dns_fallback_response_header* header, unabto_dns_header* dnsHeader, uint8_t* buffer, uint8_t* bufferEnd);
bool unabto_dns_fallback_probe_timeout_test_request(unabto_dns_fallback_session* session);


#endif
