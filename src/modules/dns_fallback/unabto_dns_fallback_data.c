#include "unabto_dns_fallback_data.h"

#include <unabto/unabto_util.h>
#include <unabto/unabto_memory.h>

#if NABTO_ENABLE_DNS_FALLBACK

void unabto_dns_fallback_data_get_response(unabto_dns_fallback_session* session, unabto_dns_fallback_response_header* header, uint8_t* begin, uint8_t* end)
{
    if (session->state == UDFS_OPEN) {
        uint32_t host;
        uint16_t port;
        uint8_t* data;
        uint16_t dataLength;
        READ_FORWARD_U32(host, begin);
        READ_FORWARD_U16(port, begin);
        begin += 2; //READ_FORWARD_U16(moreData, begin);
        data = begin;
        dataLength = header->length - 13;
        if (header->status == UDF_STATUS_OK && dataLength > 0) {
            session->recvEp.addr = host;
            session->recvEp.port = port;
            memcpy(session->recvBuffer, data, header->length);
            session->recvState = UDF_RECV_HAS_PACKET;
            session->recvLength = dataLength;
        } else if (header->status == UDF_STATUS_TIMEOUT) {
            //nabtoSetFutureStamp(&session->recvTimeout, 0);
        } else if (header->status == UDF_STATUS_INVALID_SESSION) {
            session->state = UDFS_SESSION_OPENING;
        }
    }
}

bool unabto_dns_fallback_data_send_get_request(unabto_dns_fallback_session* session)
{
    uint8_t* ptr = requestBuffer;
    uint16_t requestLength;
    uint8_t* buffer = nabtoCommunicationBuffer;
    uint8_t* end = buffer + nabtoCommunicationBufferSize;
    uint16_t packetLength;
    
    WRITE_FORWARD_U8(ptr, UDF_TYPE_GET);
    WRITE_FORWARD_U16(ptr, 9);
    WRITE_FORWARD_U16(ptr, session->sessionId);
    WRITE_FORWARD_U16(ptr, session->random++);
    WRITE_FORWARD_U16(ptr, session->recvFrequency);
    requestLength = ptr - requestBuffer;
    
    buffer = unabto_dns_fallback_encode_request_as_question(buffer, end, requestBuffer, requestLength, session->domain, &session->current_get_identification);
    if (buffer == NULL) {
        return false;
    }
    packetLength = buffer - nabtoCommunicationBuffer;
    NABTO_LOG_INFO(("Sending DNS get request id: %" PRIu16 " timeout: %" PRIu16, session->current_get_identification, session->recvFrequency));
    nabto_write(session->dnsClient.sockets[(session->dnsClient.lastUsedSocket++) % DNS_CLIENT_SOCKETS], nabtoCommunicationBuffer, packetLength, session->dnsClient.dnsServer.addr, session->dnsClient.dnsServer.port);
    return true;
}

void unabto_dns_fallback_data_send_start_response(unabto_dns_fallback_session* session, unabto_dns_fallback_response_header* header)
{
    if (session->state == UDFS_OPEN) {
        if (header->status == UDF_STATUS_INVALID_SESSION) {
            session->state = UDFS_SESSION_OPENING;
        }
    }
}

bool unabto_dns_fallback_data_send_packet_start_request(unabto_dns_fallback_session* session, uint16_t chunks, nabto_endpoint* ep, uint16_t totalLength)
{
    uint8_t requestBuffer[19];
    uint8_t* ptr = requestBuffer;
    uint8_t* buffer;
    uint8_t* end;
    uint16_t id;
    size_t length;
    
    WRITE_FORWARD_U8(ptr,  UDF_TYPE_SEND_START);
    WRITE_FORWARD_U16(ptr, 19);
    WRITE_FORWARD_U16(ptr, session->sessionId);
    WRITE_FORWARD_U16(ptr, session->random++);
    WRITE_FORWARD_U16(ptr, session->packetId);
    WRITE_FORWARD_U16(ptr, chunks);
    WRITE_FORWARD_U32(ptr, ep->addr);
    WRITE_FORWARD_U16(ptr, ep->port);
    WRITE_FORWARD_U16(ptr, totalLength);

    buffer = requestEncoded;
    end = buffer + ENCODED_BUFFER_SIZE;
    
    buffer = unabto_dns_fallback_encode_request_as_question(buffer, end, requestBuffer, 19, session->domain, &id);
    if (buffer == NULL) {
        return false;
    }
    length = buffer - requestEncoded;
    NABTO_LOG_INFO(("Sending DNS start request id: %" PRIu16, id));
    nabto_write(session->dnsClient.sockets[(session->dnsClient.lastUsedSocket++) % DNS_CLIENT_SOCKETS], requestEncoded, length, session->dnsClient.dnsServer.addr, session->dnsClient.dnsServer.port);
    return true;
}

void unabto_dns_fallback_data_send_chunk_response(unabto_dns_fallback_session* session, unabto_dns_fallback_response_header* header)
{
    if (session->state == UDFS_OPEN) {
        if (header->status == UDF_STATUS_INVALID_SESSION) {
            session->state = UDFS_SESSION_OPENING;
        }
    }
}

bool unabto_dns_fallback_data_send_packet_chunk_request(unabto_dns_fallback_session* session, uint8_t* start, uint16_t dataStart, uint16_t length, uint16_t chunkId)
{
    uint8_t* ptr = requestBuffer;
    uint16_t requestLength;
    uint8_t* buffer;
    uint8_t* end;
    uint16_t id;
    uint16_t packetLength;
    
    WRITE_FORWARD_U8(ptr, UDF_TYPE_SEND_CHUNK);
    WRITE_FORWARD_U16(ptr, length+13);
    WRITE_FORWARD_U16(ptr, session->sessionId);
    WRITE_FORWARD_U16(ptr, session->random++);
    WRITE_FORWARD_U16(ptr, session->packetId);
    WRITE_FORWARD_U16(ptr, chunkId);
    WRITE_FORWARD_U16(ptr, dataStart);

    memcpy(ptr, start, length);
    ptr += length;

    requestLength = (uint16_t)(ptr - requestBuffer);
    
    buffer = requestEncoded;
    end = buffer + ENCODED_BUFFER_SIZE;
    buffer = unabto_dns_fallback_encode_request_as_question(buffer, end, requestBuffer, requestLength, session->domain, &id);
    if (buffer == NULL) {
        return false;
    }
    packetLength = buffer - requestEncoded;
    NABTO_LOG_INFO(("Sending DNS chunk request id: %" PRIu16, id));
    nabto_write(session->dnsClient.sockets[(session->dnsClient.lastUsedSocket++) % DNS_CLIENT_SOCKETS], requestEncoded, packetLength, session->dnsClient.dnsServer.addr, session->dnsClient.dnsServer.port);
    return true;
}

#define CHUNK_SIZE 100

void unabto_dns_fallback_data_send_packet(unabto_dns_fallback_session* session, uint8_t* buffer, uint16_t bufferLength, nabto_endpoint* ep)
{
    uint16_t chunks = (bufferLength+(CHUNK_SIZE-1))/CHUNK_SIZE;
    uint16_t packetOffset = 0;
    uint16_t chunkId = 0;
    unabto_dns_fallback_data_send_packet_start_request(session, chunks, ep, bufferLength);
    while(bufferLength > 0) {
        uint16_t chunkLength = MIN(bufferLength, CHUNK_SIZE);

        unabto_dns_fallback_data_send_packet_chunk_request(session, buffer, packetOffset, chunkLength, chunkId);
        buffer += chunkLength;
        bufferLength -= chunkLength;
        packetOffset += chunkLength;
        chunkId++;
     
    }
    session->packetId++;
}

#endif
