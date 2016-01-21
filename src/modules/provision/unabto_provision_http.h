
#ifndef _UNABTO_PROVISION_HTTP_H
#define _UNABTO_PROVISION_HTTP_H

#include <stdint.h>

#include <unabto/unabto_main_contexts.h>
#include "unabto_provision.h"

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
    UPS_INTERNAL_ERROR,
    UPS_PROV_SERVICE_PROBLEM,
    UPS_PROV_ALREADY_PROVISIONED,
    UPS_PROV_INVALID_KEY,
    UPS_PROV_INVALID_TOKEN,
} unabto_provision_status_t;

#define SERVICE_URL_MAX_LENGTH 1024
#define SERVICE_TOKEN_MAX_LENGTH 40
#define SERVICE_POST_MAX_LENGTH 256

#define NABTO_PROVISION_CREATE_PATH                 "/api/1/device/provision"
#define NABTO_PROVISION_VALIDATE_PATH               "/api/1/device/validate"

/**
 * Post data to specified URL
 * @returns UPS_OK if http request was successfully invoked and http server returned status 200. Caller must free response.
 * @returns UPS_HTTP_COMPLETE_NOT_200 if http request was completed but http server returned a status != 200. Caller must free response.
 * @returns UPS_HTTP_MALFORMED_URL if specified url was not valid
 * @returns UPS_HTTP_CONNECTION_REFUSED if http server rejected TCP connection
 * @returns UPS_HTTP_OTHER if interaction with http server failed for reasons not otherwise described
 */
unabto_provision_status_t unabto_provision_http_post(const char* url, const char* data, uint16_t* http_status, char** body, const char* headers);

unabto_provision_status_t unabto_provision_http_get(const char* url, uint16_t* http_status, char** response);

unabto_provision_status_t unabto_provision_http_set_cert_path(const char* path);

unabto_provision_status_t unabto_provision_http_post_json(const char* url, const char* data, uint16_t* http_status, char** body);
unabto_provision_status_t unabto_provision_validate_key(const uint8_t* id, const uint8_t* key, provision_context_t* context);

unabto_provision_status_t unabto_provision_http(nabto_main_setup* nms, provision_context_t* context, uint8_t* key);

const char* unabto_provision_http_get_cert_path();

bool unabto_provision_parse(nabto_main_setup *nms, char *response, char *key);

void unabto_provision_http_extract_body(char** body, char* response);




#endif
