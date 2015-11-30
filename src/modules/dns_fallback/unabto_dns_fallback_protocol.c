#include "unabto_dns_fallback_protocol.h"
#include "unabto_dns_fallback_data.h"
#include "unabto_dns_fallback_session.h"
#include "unabto_dns_fallback_probe.h"

#include <modules/util/unabto_base32.h>

#include <unabto_platform_types.h>
#include <unabto/unabto_util.h>
#include <unabto/unabto_logging.h>
#include <unabto/unabto_dns_fallback.h>
#include <unabto/unabto_memory.h>

#if NABTO_ENABLE_DNS_FALLBACK

uint8_t requestBuffer[128];
uint8_t requestEncoded[ENCODED_BUFFER_SIZE];

void unabto_dns_fallback_event(const uint8_t* buffer, size_t bufferLength)
{
    
}



#if UNABTO_PLATFORM_UNIX || __unix || __unix__ || __APPLE__

void configure_native_dns_server(unabto_dns_fallback_session* session)
{
    FILE *resolvConf = fopen("/etc/resolv.conf", "r");
    if (resolvConf == NULL) {
        NABTO_LOG_ERROR(("Could not open /etc/resolv.conf"));
        return;
    }
    bool configured = false;
    char line[1024];
    while (fgets (line, sizeof(line), resolvConf)) {
        char *found = strstr(line, "nameserver ");
        if (found) {
            char *begin = line+strlen("nameserver ");
            struct in_addr inp;
            int status = inet_aton(begin, &inp);
            if (status == 1) {
                configured = true;
                session->dnsClient.dnsServer.addr = htonl(inp.s_addr);
                session->dnsClient.dnsServer.port = 53;
                NABTO_LOG_INFO(("read dns server from resolv.conf " PRIep, MAKE_EP_PRINTABLE(session->dnsClient.dnsServer) ));
                                    
            }
        }
    }
    if (!configured) {
        NABTO_LOG_ERROR(("No dns server found defaulting to 8.8.8.8"));
        session->dnsClient.dnsServer.addr = htonl(0x08080808);
        session->dnsClient.dnsServer.port = 53;
    }
    
}
#else
void configure_native_dns_server(unabto_dns_fallback_session* session)
{
    bool configured = false;
    if (!configured) {
        NABTO_LOG_ERROR(("No dns server found defaulting to 8.8.8.8"));
        session->dnsClient.dnsServer.addr = 0x08080808;
        session->dnsClient.dnsServer.port = 53;
    }
}
#endif

void unabto_dns_fallback_open(unabto_dns_fallback_session* session)
{
    NABTO_LOG_INFO(("unabto_dns_fallback_open"));
    if (nmc.nabtoMainSetup.dnsFallbackDomain != NULL) {
        session->domain = nmc.nabtoMainSetup.dnsFallbackDomain;
        session->state = UDFS_SESSION_OPENING;
    } else {
        session->state = UDFS_RESOLVE_DOMAIN;
    }
    if (nmc.nabtoMainSetup.dnsAddress != INADDR_NONE) {
        session->dnsClient.dnsServer.addr = nmc.nabtoMainSetup.dnsAddress;
        session->dnsClient.dnsServer.port = 53;
    } else {
        configure_native_dns_server(session);
    }
    nabto_random((uint8_t*)&session->random, sizeof(session->random));
    session->packetId = 0;
    nabtoSetFutureStamp(&session->timeout, 0);
    session->recvFrequency = UDFP_TIMEOUT_TEST_MIN_TIMEOUT;
    session->recvTimeoutTime = 10000;
}

void unabto_dns_fallback_stop(unabto_dns_fallback_session* session)
{
    session->state = UDFS_IDLE;
}

void unabto_dns_fallback_timeout(unabto_dns_fallback_session* session)
{
    if (session->state == UDFS_IDLE) {
        return;
    }
    if (session->state == UDFS_RESOLVE_DOMAIN) {
        if (nabtoIsStampPassed(&session->timeout)) {
            unabto_dns_lookup_domain(session);
            nabtoSetFutureStamp(&session->timeout, 10000);
        }
    } 

    if (session->state == UDFS_SESSION_OPENING) {
        if (nabtoIsStampPassed(&session->timeout)) {
            unabto_dns_fallback_session_send_open_request(session);
            nabtoSetFutureStamp(&session->timeout, 10000);
        }
    }
    if (session->state == UDFS_OPEN) {
        if (nabtoIsStampPassed(&session->timeout)) {
            unabto_dns_fallback_data_send_get_request(session);
            nabtoSetFutureStamp(&session->timeout, session->recvTimeoutTime);
        }
    }

    unabto_dns_fallback_probe_event(session);

    return;
}

void unabto_dns_fallback_packet(unabto_dns_fallback_session* session, uint8_t* buffer, uint16_t bufferSize)
{
    uint8_t* end = buffer + bufferSize;
    unabto_dns_header dnsHeader;
    uint16_t errorCode;
    int i;
    buffer = unabto_dns_header_parse(&dnsHeader, buffer, end);

    errorCode = unabto_dns_header_response_code(&dnsHeader);

    if (errorCode != 0) {
        if (errorCode == 2 && dnsHeader.identification == session->current_get_identification) {
            NABTO_LOG_ERROR(("Dns error on current get request id: %" PRIu16, dnsHeader.identification));
            nabtoSetFutureStamp(&session->timeout, 0);
            return;
        }
        NABTO_LOG_ERROR(("Dns error %" PRIu16 " id: %" PRIu16, errorCode, dnsHeader.identification));
        return;
    }

    NABTO_LOG_INFO(("Dns answer id %" PRIu16, dnsHeader.identification));
    
    //unabto_dns_print_header(&dnsHeader);
    
    for (i = 0; i < dnsHeader.questionCount; i++) {
        buffer = unabto_dns_skip_question(buffer, end);
        if (buffer == NULL) { return; }
    }

    for (i = 0; i < dnsHeader.answerCount; i++) {
        uint16_t type;
        uint16_t rdataLength;
        uint8_t* rdataEnd;
        buffer = unabto_dns_skip_name(buffer, end);
        if (buffer == NULL) { return; }

        READ_FORWARD_U16(type, buffer);
        buffer += 6;
        READ_FORWARD_U16(rdataLength, buffer);
        rdataEnd = buffer + rdataLength;
        if (type == TYPE_TXT) {


            // remove length prefixes
            uint8_t* ptr = buffer;
            uint8_t* newEnd = buffer;
            while(ptr < rdataEnd) {
                uint8_t length;
                READ_FORWARD_U8(length, ptr);
                if (ptr + length > rdataEnd) {
                    // invalid txt record
                    break;
                }
                memmove(newEnd, ptr, length);
                newEnd+=length;
                ptr+=length;
            }
            
            NABTO_LOG_TRACE(("TXT record %.*s", (int)(newEnd-buffer), buffer));

            if (session->state == UDFS_RESOLVE_DOMAIN) {
                if (rdataLength-1 > strlen("dnsfb=")) {
                    unabto_dns_fallback_handle_dnsfb(session, buffer + strlen("dnsfb="), newEnd);
                }
            }
            
            if (rdataLength-1 > strlen("NabtoAns=")) {
                if (memcmp(buffer, "NabtoAns=", strlen("NabtoAns=")) == 0) {
                    unabto_dns_fallback_handle_base32_payload(session, &dnsHeader, buffer + strlen("NabtoAns="), newEnd);
                }
            }
        } else {
            NABTO_LOG_INFO(("non TXT record %i", type));
        }
        
        buffer = rdataEnd;
    }
    if (session->state == UDFS_OPEN && dnsHeader.identification == session->current_get_identification && session->recvState == UDF_RECV_IDLE) {
        nabtoSetFutureStamp(&session->timeout, 0);
    }
}

void unabto_dns_fallback_handle_base32_payload(unabto_dns_fallback_session* session, unabto_dns_header* dnsHeader, uint8_t* begin, uint8_t* end)
{
    uint8_t* decodeEnd = unabto_base32_decode(begin, end, begin, end);

    size_t length = decodeEnd - begin;
    
    unabto_dns_fallback_response_header header;
    begin = unabto_dns_fallback_read_response_header(&header, begin, decodeEnd);
    
    NABTO_LOG_INFO(("data length: %" PRIsize ", packet type %" PRIu8 ", packet length %" PRIu16 ", status %" PRIu16, length, header.type, header.length, header.status));

    switch (header.type) {
        case UDF_TYPE_OPEN:
            unabto_dns_fallback_session_open_response(session, &header, begin, decodeEnd);
            break;
        case UDF_TYPE_GET:
            unabto_dns_fallback_data_get_response(session, &header, begin, decodeEnd);
            break;
        case UDF_TYPE_SEND_START:
            unabto_dns_fallback_data_send_start_response(session, &header);
            break;
        case UDF_TYPE_SEND_CHUNK:
            unabto_dns_fallback_data_send_chunk_response(session, &header);
            break;
        case UDF_TYPE_TIMEOUT_TEST:
            unabto_dns_fallback_probe_timeout_test_response(session, &header, dnsHeader, begin, decodeEnd);
            break;
    }
}

uint8_t* unabto_dns_skip_question(uint8_t* buffer, uint8_t* end)
{
    buffer = unabto_dns_skip_name(buffer, end);
    if (buffer == NULL) { return NULL; }
    return buffer + 4;
}

uint8_t* unabto_dns_skip_name(uint8_t* buffer, uint8_t* end)
{
    while (*buffer != 0) {
        if ((*buffer & 0xc0) == 0xc0) {
            // label;
            buffer +=2;
            return buffer;
        } else {
            uint8_t offset = *buffer;
            buffer += offset+1;
        }
        if (buffer > end) {
            NABTO_LOG_ERROR(("dns error"));
            return NULL;
        }
    }
    return buffer + 1;
}


/**
 * Convert request into base32 encoded dns txt query.
 */
uint8_t* unabto_dns_fallback_encode_request_as_question(uint8_t* buffer, uint8_t* end, uint8_t* request, uint16_t requestLength, const char* domain, uint16_t* identification)
{
    unabto_dns_header header;
    unabto_dns_make_query_header(&header);
    *identification = header.identification;
    buffer = unabto_dns_encode_header(&header, buffer, end);
    if (buffer == NULL) {
        NABTO_LOG_ERROR(("dns request header encoding failed"));
        return NULL;
    }
    
    while(requestLength > 0) {
        // max chunk length is 39
        uint8_t chunk_length = MIN(requestLength, 39);
        uint8_t labelLength = (((uint16_t)chunk_length)*8+4)/5;
        *buffer = labelLength;
        buffer++;
        buffer = unabto_base32_encode(buffer, end, request, request+chunk_length);
        requestLength -= chunk_length;
        request += chunk_length;
    }
    buffer = unabto_dns_encode_name(buffer, end, domain);
    WRITE_FORWARD_U8(buffer, 0); // terminating zero
    WRITE_FORWARD_U16(buffer, TYPE_TXT);
    WRITE_FORWARD_U16(buffer, CLASS_IN);
    return buffer;
}

bool unabto_dns_fallback_can_send_packet(unabto_dns_fallback_session* session)
{
    if (session->state == UDFS_OPEN) {
        return true;
    }
    return false;
}

uint16_t unabto_dns_recv_packet(unabto_dns_fallback_session* session, uint8_t* buffer, uint16_t bufferLength, nabto_endpoint* ep)
{
    if(session->recvState == UDF_RECV_HAS_PACKET) {
        uint16_t packetLength = MIN(session->recvLength, bufferLength);
        memcpy(buffer, session->recvBuffer, packetLength);
        *ep = session->recvEp;
        session->recvState = UDF_RECV_IDLE;
        nabtoSetFutureStamp(&session->timeout, 0);
        return packetLength;
    }
    return 0;
}

void unabto_dns_lookup_domain(unabto_dns_fallback_session* session)
{
    const char* domain = nmc.nabtoMainSetup.id;
    unabto_dns_header header;
    uint8_t* start = nabtoCommunicationBuffer;
    uint8_t* buffer = start;
    uint8_t* end = buffer+nabtoCommunicationBufferSize;
    size_t length;
    unabto_dns_make_query_header(&header);
    buffer = unabto_dns_encode_header(&header, buffer, end);
    if (buffer == NULL) {
        NABTO_LOG_ERROR(("dns request header encoding failed"));
        return;
    }
    
    buffer = unabto_dns_encode_name(buffer, end, domain);
    if (buffer == NULL) {
        NABTO_LOG_ERROR(("dns request domain encoding failed"));
        return;
    }

    if (end-buffer < 5) {
        NABTO_LOG_ERROR(("Buffer too small"));
        return;
    }
    WRITE_FORWARD_U8(buffer, 0); // terminating zero
    WRITE_FORWARD_U16(buffer, TYPE_TXT);
    WRITE_FORWARD_U16(buffer, CLASS_IN);

    length = buffer - start;
    NABTO_LOG_INFO(("Sending txt lookup on %s", domain));
    nabto_write(session->dnsClient.sockets[(session->dnsClient.lastUsedSocket++) % DNS_CLIENT_SOCKETS], start, length, session->dnsClient.dnsServer.addr, session->dnsClient.dnsServer.port);
}

void unabto_dns_fallback_handle_dnsfb(unabto_dns_fallback_session* session, uint8_t* buffer, uint8_t* end)
{
    size_t length;
    if (buffer > end) {
        return;
    }
    length = end - buffer;
    if (length > (UNABTO_DNS_DOMAIN_LENGTH - 1)) {
        return;
    }
    memset(session->domainBuffer, 0, UNABTO_DNS_DOMAIN_LENGTH);
    memcpy(session->domainBuffer, buffer, length);
    session->domain = session->domainBuffer;
    session->state = UDFS_SESSION_OPENING;
    nabtoSetFutureStamp(&session->timeout, 0);
    return;
    
}




uint8_t* unabto_dns_fallback_read_response_header(unabto_dns_fallback_response_header* header, uint8_t* begin, uint8_t* end)
{
    if (begin == NULL) {
        return NULL;
    }
    if (end < begin) {
        return NULL;
    }
    if (end-begin < 5) {
        return NULL;
    }
    READ_FORWARD_U8(header->type, begin);
    READ_FORWARD_U16(header->length, begin);
    READ_FORWARD_U16(header->status, begin);
    return begin;
}


#endif
