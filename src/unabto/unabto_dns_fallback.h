#ifndef _UNABTO_DNS_FALLBACK_H_
#define _UNABTO_DNS_FALLBACK_H_

#include <unabto_platform_types.h>
#include <unabto/unabto_env_base.h>

typedef enum {
    UDF_OK,
    UDF_SOCKET_CREATING,
    UDF_SOCKET_CREATE_FAILED
} unabto_dns_fallback_error_code;

bool unabto_dns_fallback_init();
bool unabto_dns_fallback_close();

void unabto_dns_fallback_handle_packet(uint8_t* buffer, uint16_t bufferLength);
void unabto_dns_fallback_handle_timeout();
void unabto_dns_fallback_next_event(nabto_stamp_t* current_min_stamp);
uint16_t unabto_dns_fallback_recv_socket(uint8_t* buffer, uint16_t bufferLength);
    
unabto_dns_fallback_error_code unabto_dns_fallback_create_socket();
bool unabto_dns_fallback_close_socket();
uint16_t unabto_dns_fallback_send_to(uint8_t* buf, uint16_t bufSize, uint32_t addr, uint16_t port);
uint16_t unabto_dns_fallback_recv_from(uint8_t* buf, uint16_t bufferSize, uint32_t* addr, uint16_t* port);

#endif
