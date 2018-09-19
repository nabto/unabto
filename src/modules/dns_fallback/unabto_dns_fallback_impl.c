#include <unabto/unabto_dns_fallback.h>
#include "unabto_dns_fallback_protocol.h"
#include "unabto_dns_fallback_data.h"
#include <unabto/unabto_next_event.h>
#include <unabto/unabto_memory.h>

#if NABTO_ENABLE_DNS_FALLBACK

unabto_dns_fallback_session session;

bool unabto_dns_fallback_init()
{
    memset(&session, 0, sizeof(unabto_dns_fallback_session));
    session.state = UDFS_IDLE;
    return true;
}

bool unabto_dns_fallback_close()
{
    return true;
}

uint16_t unabto_dns_fallback_recv_socket(uint8_t* buffer, uint16_t bufferLength) {
    struct nabto_ip_address addr;
    uint16_t port;
    size_t ilen;
    int i;
    for (i = 0; i < DNS_CLIENT_SOCKETS; i++) {
        ilen = nabto_read(session.dnsClient.sockets[i], buffer, bufferLength, &addr, &port);
        if (ilen > 0) {
            return (uint16_t)ilen;
        }
    }
    return 0;
}

void unabto_dns_fallback_handle_packet(uint8_t* buffer, uint16_t bufferLength)
{
    unabto_dns_fallback_packet(&session, buffer, bufferLength);
}

void unabto_dns_fallback_handle_timeout() {
    unabto_dns_fallback_timeout(&session);
}

void unabto_dns_fallback_next_event(nabto_stamp_t* current_min_stamp)
{
    if (session.state == UDFS_RESOLVE_DOMAIN || session.state == UDFS_SESSION_OPENING || session.state == UDFS_OPEN) {
        nabto_update_min_stamp(current_min_stamp, &session.timeout);
    }
}


unabto_dns_fallback_error_code unabto_dns_fallback_create_socket()
{
    if (session.state == UDFS_IDLE) {
        
        int i;
        for (i = 0; i < DNS_CLIENT_SOCKETS; i++)  {
            uint16_t localPort = 0;
            nabto_init_socket(&localPort, &session.dnsClient.sockets[i]);
        }
        unabto_dns_fallback_open(&session);
    }
    if (session.state == UDFS_OPEN) {
        return UDF_OK;
    }
    if (session.state == UDFS_SESSION_OPENING || session.state == UDFS_RESOLVE_DOMAIN) {
        return UDF_SOCKET_CREATING;
    }
    return UDF_SOCKET_CREATE_FAILED;
}

bool unabto_dns_fallback_close_socket()
{
    if (session.state != UDFS_IDLE) {
        int i;
        for (i = 0; i < DNS_CLIENT_SOCKETS; i++) {
            nabto_close_socket(&session.dnsClient.sockets[i]);
        }
        unabto_dns_fallback_stop(&session);
    }
    return true;
}
uint16_t unabto_dns_fallback_send_to(uint8_t* buf, uint16_t bufSize, uint32_t addr, uint16_t port)
{
    nabto_endpoint ep;
    ep.addr = addr;
    ep.port = port;
    unabto_dns_fallback_data_send_packet(&session, buf, bufSize, &ep);
    return bufSize;
}

uint16_t unabto_dns_fallback_recv_from(uint8_t* buf, uint16_t bufferSize, uint32_t* addr, uint16_t* port)
{
    nabto_endpoint ep;
    uint16_t l;
    l = unabto_dns_recv_packet(&session, buf, bufferSize, &ep);
    if (l > 0) {
        *addr = ep.addr;
        *port = ep.port;
    }
    return l;
}

#endif
