/**
 * The probe module is used to probe for features and limits in the dns communication
 */

#include "unabto_dns_fallback_probe.h"
#include <unabto/unabto_memory.h>
#include <unabto/unabto_util.h>

#if NABTO_ENABLE_DNS_FALLBACK

void unabto_dns_fallback_probe_start_timeout_test(unabto_dns_fallback_session* session)
{
    session->probe.state = UDF_PROBE_DETECT_TIMEOUT;
    session->probe.currentTimeout = UDFP_TIMEOUT_TEST_MIN_TIMEOUT;
    session->probe.retries = 0;
    nabtoSetFutureStamp(&session->probe.timeout, 0);
}

void unabto_dns_fallback_probe_event(unabto_dns_fallback_session* session) {
    unabto_dns_fallback_probe* probe = &session->probe;
    if (probe->state == UDF_PROBE_DETECT_TIMEOUT) {
        if (nabtoIsStampPassed(&probe->timeout)) {
            if (probe->retries > UDFP_TIMEOUT_TEST_MAX_RETRIES) {
                NABTO_LOG_TRACE(("Adaptive dynamic timeout test stopped, ended with timeout: %" PRIu16 ", timeout in use: %" PRIu16, probe->currentTimeout, session->recvTimeoutTime));
                probe->state = UDF_PROBE_IDLE;
            }
            
            unabto_dns_fallback_probe_timeout_test_request(session);
            probe->retries++;
            nabtoSetFutureStamp(&session->probe.timeout, session->probe.currentTimeout*2);
        }
    }
}

void unabto_dns_fallback_probe_timeout_test_response(unabto_dns_fallback_session* session, unabto_dns_fallback_response_header* header, unabto_dns_header* dnsHeader, uint8_t* buffer, uint8_t* bufferEnd)
{
    unabto_dns_fallback_probe* probe = &session->probe;
    if (probe->state == UDF_PROBE_DETECT_TIMEOUT && dnsHeader->identification == probe->identification) {
        uint16_t newProposedTimeout;
        probe->identification = 0;
        // Perfect we got an answer to a timeout test requests.
        newProposedTimeout = probe->currentTimeout/2;
        if (newProposedTimeout > session->recvFrequency) {
            NABTO_LOG_TRACE(("Adjusting recvtimeout time to %" PRIu16, newProposedTimeout));
            session->recvFrequency = newProposedTimeout;
        }
        if (probe->currentTimeout*2 >= UDFP_TIMEOUT_TEST_MAX_TIMEOUT) {
            probe->state = UDF_PROBE_IDLE;
        } else {
            probe->currentTimeout *= 2;
            probe->retries = 0;
            nabtoSetFutureStamp(&probe->timeout, 0);
        }
    }
}

bool unabto_dns_fallback_probe_timeout_test_request(unabto_dns_fallback_session* session)
{
    unabto_dns_fallback_probe* probe = &session->probe;

    uint8_t* ptr = requestBuffer;
    uint8_t* buffer = nabtoCommunicationBuffer;
    uint8_t* end = buffer + nabtoCommunicationBufferSize;
    size_t requestLength = ptr - requestBuffer;
    size_t packetLength;
    
    WRITE_FORWARD_U8(ptr, UDF_TYPE_TIMEOUT_TEST);
    WRITE_FORWARD_U16(ptr, 9);
    WRITE_FORWARD_U16(ptr, session->sessionId);
    WRITE_FORWARD_U16(ptr, session->random++);
    WRITE_FORWARD_U16(ptr, session->probe.currentTimeout);
    
    buffer = unabto_dns_fallback_encode_request_as_question(buffer, end, requestBuffer, requestLength, session->domain, &probe->identification);

    if (buffer == NULL) {
        return false;
    }
    
    packetLength = buffer - nabtoCommunicationBuffer;
    NABTO_LOG_INFO(("Sending timeout test request id: %" PRIu16 " timeout: %" PRIu16, probe->identification, probe->currentTimeout));
    nabto_write(session->dnsClient.sockets[(session->dnsClient.lastUsedSocket++) % DNS_CLIENT_SOCKETS], nabtoCommunicationBuffer, packetLength, session->dnsClient.dnsServer.addr, session->dnsClient.dnsServer.port);
    return true;
}

#endif
