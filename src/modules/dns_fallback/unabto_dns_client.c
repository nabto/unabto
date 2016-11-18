#include "unabto_dns_client.h"

#include <unabto/unabto_memory.h>
#include <unabto/unabto_util.h>
#include <unabto/unabto_logging.h>

uint8_t* unabto_dns_header_parse(unabto_dns_header* header, uint8_t* data, uint8_t* end)
{
    if (end - data < 12) {
        NABTO_LOG_ERROR(("dns header too short"));
        return NULL;
    }
    READ_FORWARD_U16(header->identification, data);
    READ_FORWARD_U16(header->flags, data);
    READ_FORWARD_U16(header->questionCount, data);
    READ_FORWARD_U16(header->answerCount, data);
    READ_FORWARD_U16(header->authorityCount, data);
    READ_FORWARD_U16(header->additionalCount, data);
    return data;
}

uint16_t unabto_dns_header_response_code(unabto_dns_header* header)
{
    return header->flags & FLAG_RESPONSE_CODE_MASK;
}

void unabto_dns_print_header(unabto_dns_header* header) {
    NABTO_LOG_INFO(("identification %" PRIu16, header->identification));
    NABTO_LOG_INFO(("flags %" PRIu16, header->flags));
    if (header->flags & FLAG_RESPONSE_CODE_MASK) {
        NABTO_LOG_INFO(("flags indicates error %" PRIu16, (uint16_t)(header->flags & FLAG_RESPONSE_CODE_MASK)));
    }

    NABTO_LOG_INFO(("questionCount %" PRIu16, header->questionCount));
    NABTO_LOG_INFO(("answerCount %" PRIu16, header->answerCount));
    NABTO_LOG_INFO(("authorityCount %" PRIu16, header->authorityCount));
    NABTO_LOG_INFO(("additionalCount %" PRIu16, header->additionalCount));
}

void unabto_dns_make_query_header(unabto_dns_header* header)
{
    memset(header, 0 , sizeof(unabto_dns_header));
    header->identification = rand();
    header->flags = FLAG_OPCODE_STANDARD_QUERY | FLAG_RECURSION_DESIRED;
    header->questionCount = 1;
    header->answerCount = 0;
    header->authorityCount = 0;
    header->additionalCount = 0;
}

uint8_t* unabto_dns_encode_header(unabto_dns_header* header, uint8_t* buffer, uint8_t* end)
{
    WRITE_FORWARD_U16(buffer, header->identification);
    WRITE_FORWARD_U16(buffer, header->flags);
    WRITE_FORWARD_U16(buffer, header->questionCount);
    WRITE_FORWARD_U16(buffer, header->answerCount);
    WRITE_FORWARD_U16(buffer, header->authorityCount);
    WRITE_FORWARD_U16(buffer, header->additionalCount);
    return buffer;
}

uint8_t* unabto_dns_encode_name(uint8_t* buffer, uint8_t* end, const char* name)
{
    uint8_t* labelLengthPointer;
    while(*name != 0) {
        labelLengthPointer = buffer++;
        *labelLengthPointer = 0;
        while(*name != '.' && *name != 0) {
            WRITE_FORWARD_U8(buffer, *name++);
            *labelLengthPointer = *labelLengthPointer + 1;
        }
        
        if(*name == '.') {
            name++;
        }
    }
    return buffer;
}

uint8_t* unabto_dns_encode_query(uint16_t qtype, const char* name, uint8_t* buffer, uint8_t* end)
{
    buffer = unabto_dns_encode_name(buffer, end, name);

    WRITE_FORWARD_U8(buffer, 0); // terminating zero
    WRITE_FORWARD_U16(buffer, qtype);
    WRITE_FORWARD_U16(buffer, CLASS_IN);
    return buffer;
}

void unabto_dns_send_query(unabto_dns_session* session, uint16_t qtype, const char* domain)
{
    uint8_t* buffer = nabtoCommunicationBuffer;
    uint8_t* end = nabtoCommunicationBuffer + nabtoCommunicationBufferSize;
    unabto_dns_header header;
    size_t length;
    unabto_dns_make_query_header(&header);
    buffer = unabto_dns_encode_header(&header, buffer, end);
    if (buffer == NULL) {
        NABTO_LOG_ERROR(("dns request header encoding failed"));
        return;
    }
    buffer = unabto_dns_encode_query(qtype, domain, buffer, end);
    if (buffer == NULL) {
        NABTO_LOG_ERROR(("dns request query encoding failed"));
        return;
    }
    length = buffer - nabtoCommunicationBuffer;
    nabto_write(session->sockets[(session->lastUsedSocket++) % DNS_CLIENT_SOCKETS], nabtoCommunicationBuffer, length, session->dnsServer.addr, session->dnsServer.port);
}
