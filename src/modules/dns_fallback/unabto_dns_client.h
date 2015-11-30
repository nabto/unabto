#ifndef _UNABTO_DNS_CLIENT_H_
#define _UNABTO_DNS_CLIENT_H_

#include <unabto_platform_types.h>
#include <unabto/unabto_external_environment.h>
#include <unabto/unabto_context.h>


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
#define TYPE_TXT                                                    0x0010

#define CLASS_IN                                                    0x0001

typedef struct {
    uint16_t identification;
    uint16_t flags;
    uint16_t questionCount;
    uint16_t answerCount;
    uint16_t authorityCount;
    uint16_t additionalCount;
} unabto_dns_header;

enum {
    DNS_CLIENT_SOCKETS = 1
};

typedef struct {
    nabto_socket_t sockets[DNS_CLIENT_SOCKETS];
    nabto_endpoint dnsServer;
    uint8_t lastUsedSocket;
} unabto_dns_session;

typedef enum {
    UDRS_IDLE,
    UDRS_RESOLVING,
    UDRS_RESOLVED
} unabto_dns_resolve_state;

uint8_t* unabto_dns_header_parse(unabto_dns_header* header, uint8_t* buffer, uint8_t* end);
uint16_t unabto_dns_header_response_code(unabto_dns_header* header);
void unabto_dns_send_query(unabto_dns_session* session, uint16_t qtype, const char* name);

void unabto_dns_make_query_header(unabto_dns_header* header);

uint8_t* unabto_dns_encode_header(unabto_dns_header* header, uint8_t* buffer, uint8_t* end);
uint8_t* unabto_dns_encode_query(uint16_t qtype, const char* name, uint8_t* buffer, uint8_t* end);
void unabto_dns_print_header(unabto_dns_header* header);
uint8_t* unabto_dns_encode_name(uint8_t* buffer, uint8_t* end, const char* name);

#endif
