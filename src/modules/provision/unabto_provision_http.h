#ifndef _UNABTO_PROVISION_HTTP_H
#define _UNABTO_PROVISION_HTTP_H

#include <stdint.h>

#include <src/unabto/unabto_main_contexts.h>

#define KEY_BUFFER_SIZE PRE_SHARED_KEY_SIZE*2+1 
#define HTTP_MAX_POST_LENGTH 256

typedef enum {
    UPS_OK,
    UPS_HTTP_COMPLETE_NOT_200,
    UPS_HTTP_MALFORMED_URL,
    UPS_HTTP_CONNECTION_REFUSED,
    UPS_HTTP_OTHER,
    UPS_HTTP_BAD_CACERTFILE,
    UPS_HTTP_SSL_PROBLEM,
    UPS_INTERNAL_ERROR
} unabto_provision_status_t;

#define SERVICE_URL_MAX_LENGTH 1024
#define SERVICE_TOKEN_MAX_LENGTH 40
#define SERVICE_POST_MAX_LENGTH 256

typedef struct {
    const uint8_t* scheme_;
    const uint8_t* host_;
    const uint8_t* api_key_;
    const uint8_t* token_;
    const uint8_t* id_;
    uint8_t* key_;
} provision_context_t;

/**
 * Post data to specified URL
 * @returns UPS_OK if http request was successfully invoked and http server returned status 200. Caller must free response.
 * @returns UPS_HTTP_COMPLETE_NOT_200 if http request was completed but http server returned a status != 200. Caller must free response.
 * @returns UPS_HTTP_MALFORMED_URL if specified url was not valid
 * @returns UPS_HTTP_CONNECTION_REFUSED if http server rejected TCP connection
 * @returns UPS_HTTP_OTHER if interaction with http server failed for reasons not otherwise described
 */
unabto_provision_status_t unabto_provision_http_post(const char* url, const char* data, uint16_t* http_status, char** body, const char* headers);

unabto_provision_status_t unabto_provision_http_set_cert_path(const char* path);

const char* unabto_provision_http_get_cert_path();


#endif
