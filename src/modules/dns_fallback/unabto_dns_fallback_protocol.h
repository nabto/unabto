#ifndef _UNABTO_DNS_FALLBACK_PROTOCOL_H_
#define _UNABTO_DNS_FALLBACK_PROTOCOL_H_

#include "unabto_dns_client.h"

#include <unabto_platform_types.h>
#include <unabto/unabto_external_environment.h>
#include <unabto/unabto_context.h>

enum {
    ENCODED_BUFFER_SIZE = 12+255+2+2
};

extern uint8_t requestBuffer[128];
extern uint8_t requestEncoded[ENCODED_BUFFER_SIZE];

enum {
    UDF_TYPE_OPEN = 'o',
    UDF_TYPE_CLOSE = 'c',
    UDF_TYPE_SEND_START = 's',
    UDF_TYPE_SEND_CHUNK = 'h',
    UDF_TYPE_GET = 'g',
    UDF_TYPE_TIMEOUT_TEST = 't'
};

enum {
    UDF_STATUS_OK = 0,
    UDF_STATUS_TIMEOUT = 1,
    UDF_STATUS_CLIENT_ERROR = 2,
    UDF_STATUS_SERVER_ERROR = 3,
    UDF_STATUS_INVALID_SESSION = 4
};

typedef enum {
    UDFS_IDLE,
    UDFS_RESOLVE_DOMAIN,
    UDFS_SESSION_OPENING,
    UDFS_OPEN,
    UDFS_CLOSING,
    UDFS_CLOSED
} unabto_dns_fallback_state;

typedef enum {
    UDF_PROBE_IDLE,
    UDF_PROBE_DETECT_TIMEOUT
} unabto_dns_fallback_probe_state;

typedef struct {
    unabto_dns_fallback_probe_state state;
    nabto_stamp_t timeout;
    uint16_t currentTimeout;
    uint16_t identification;
    uint8_t retries;
} unabto_dns_fallback_probe;

typedef enum {
    UDF_RECV_IDLE,
    UDF_RECV_RECEIVING,
    UDF_RECV_HAS_PACKET
} unabto_dns_fallback_recv_state;

enum {
    UNABTO_DNS_FALLBACK_SEND_BUFFER_SIZE = 500,
    UNABTO_DNS_FALLBACK_RECV_BUFFER_SIZE = 500
};

enum {
    UNABTO_DNS_DOMAIN_LENGTH = 64
};

typedef struct {
    unabto_dns_session dnsClient;
    unabto_dns_fallback_state state;
    uint16_t random;
    const char* domain;
    char domainBuffer[UNABTO_DNS_DOMAIN_LENGTH];
    uint16_t sessionId;
    nabto_stamp_t timeout;
    uint8_t retryCount;

    uint16_t packetId;

    unabto_dns_fallback_recv_state recvState;
    uint8_t recvBuffer[UNABTO_DNS_FALLBACK_RECV_BUFFER_SIZE];
    uint16_t recvLength;
    nabto_endpoint recvEp;

    uint16_t recvTimeoutTime;
    uint16_t recvFrequency;
    uint16_t current_get_identification;
    unabto_dns_fallback_probe probe;
} unabto_dns_fallback_session;

typedef struct {
    uint8_t type;
    uint16_t length;
    uint16_t random;
} unabto_dns_fallback_request_header;

typedef struct {
    uint8_t type;
    uint16_t length;
    uint16_t status;
} unabto_dns_fallback_response_header;

void unabto_dns_fallback_packet(unabto_dns_fallback_session* session, uint8_t* buffer, uint16_t length);

void unabto_dns_fallback_timeout(unabto_dns_fallback_session* session);

uint8_t* unabto_dns_fallback_encode_request_as_question(uint8_t* buffer, uint8_t* end, uint8_t* request, uint16_t requestLength, const char* domain, uint16_t* identification);

uint8_t* unabto_dns_skip_question(uint8_t* buffer, uint8_t* end);
uint8_t* unabto_dns_skip_name(uint8_t* buffer, uint8_t* end);
void unabto_dns_fallback_handle_base32_payload(unabto_dns_fallback_session* session, unabto_dns_header* dnsHeader, uint8_t* begin, uint8_t* end);

void unabto_dns_fallback_open(unabto_dns_fallback_session* session);
void unabto_dns_fallback_stop(unabto_dns_fallback_session* session);

bool unabto_dns_fallback_can_send_packet(unabto_dns_fallback_session* session);

void unabto_dns_fallback_send_packet(unabto_dns_fallback_session* session, uint8_t* data, uint16_t dataLength, nabto_endpoint* ep);

bool unabto_dns_send_get_request(unabto_dns_fallback_session* session);
uint16_t unabto_dns_recv_packet(unabto_dns_fallback_session* session, uint8_t* buffer, uint16_t bufferLength, nabto_endpoint* ep);


void unabto_dns_lookup_domain(unabto_dns_fallback_session* session);
void unabto_dns_fallback_handle_dnsfb(unabto_dns_fallback_session* session, uint8_t* buffer, uint8_t* end);

uint8_t* unabto_dns_fallback_read_response_header(unabto_dns_fallback_response_header* header, uint8_t* begin, uint8_t* end);


#endif
