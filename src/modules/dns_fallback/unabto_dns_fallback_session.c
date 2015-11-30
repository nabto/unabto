#include "unabto_dns_fallback_session.h"
#include "unabto_dns_fallback_probe.h"

#include <unabto/unabto_memory.h>
#include <unabto/unabto_util.h>

#if NABTO_ENABLE_DNS_FALLBACK

void unabto_dns_fallback_session_send_open_request(unabto_dns_fallback_session* session)
{
    uint8_t requestBuffer[7];
    uint8_t* ptr = requestBuffer;
    uint8_t* buffer;
    uint8_t* end;
    uint16_t id;
    size_t length;
    WRITE_FORWARD_U8(ptr, UDF_TYPE_OPEN);
    WRITE_FORWARD_U16(ptr, 7);
    WRITE_FORWARD_U16(ptr, 0);
    WRITE_FORWARD_U16(ptr, session->random++);

    buffer = nabtoCommunicationBuffer;
    end = buffer+nabtoCommunicationBufferSize;
    buffer = unabto_dns_fallback_encode_request_as_question(buffer, end, requestBuffer, 7, session->domain, &id);
    if (buffer == NULL) {
        NABTO_LOG_ERROR(("dns request encode failed"));
        return;
    }
    length = buffer - nabtoCommunicationBuffer;
    session->state = UDFS_SESSION_OPENING;
    NABTO_LOG_INFO(("Sending DNS open request id: %" PRIu16, id));
    nabto_write(session->dnsClient.sockets[(session->dnsClient.lastUsedSocket++) % DNS_CLIENT_SOCKETS], nabtoCommunicationBuffer, length, session->dnsClient.dnsServer.addr, session->dnsClient.dnsServer.port);

}

void unabto_dns_fallback_session_open_response(unabto_dns_fallback_session* session, unabto_dns_fallback_response_header* response, uint8_t* buffer, uint8_t* end)
{
    if (session->state == UDFS_SESSION_OPENING) {
        uint16_t sessionId;
        READ_FORWARD_U16(sessionId, buffer);
        NABTO_LOG_INFO(("open with status %i, and sessionid %i", response->status, sessionId));
        if (response->status == UDF_STATUS_OK) {
            session->sessionId = sessionId;
            session->state = UDFS_OPEN;
            unabto_dns_fallback_probe_start_timeout_test(session);
            nabtoSetFutureStamp(&session->timeout, 0);
            NABTO_LOG_INFO(("state changed to open"));
        }
    }
}

#endif
