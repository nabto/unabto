#ifndef _UNABTO_PROVISION_HTTP_H
#define _UNABTO_PROVISION_HTTP_H

#include <stdint.h>

#define HTTP_MAX_POST_LENGTH 256

typedef enum {
    UPHS_OK,
    UPHS_HTTP_COMPLETE_NOT_200,
    UPHS_HTTP_MALFORMED_URL,
    UPHS_HTTP_CONNECTION_REFUSED,
    UPHS_HTTP_OTHER,
    UPHS_HTTP_BAD_CACERTFILE,
    UPHS_HTTP_SSL_PROBLEM
} unabto_provision_http_status_t;

/**
 * Post data to specified URL
 * @returns UPHS_OK if http request was successfully invoked and http server returned status 200. Caller must free response.
 * @returns UPHS_HTTP_COMPLETE_NOT_200 if http request was completed but http server returned a status != 200. Caller must free response.
 * @returns UPHS_HTTP_MALFORMED_URL if specified url was not valid
 * @returns UPHS_HTTP_CONNECTION_REFUSED if http server rejected TCP connection
 * @returns UPHS_HTTP_OTHER if interaction with http server failed for reasons not otherwise described
 */
unabto_provision_http_status_t unabto_provision_http_post(const char* url, const char* data, uint16_t* http_status, char** body, const char* headers);

unabto_provision_http_status_t unabto_provision_http_set_cert_path(const char* path);

const char* unabto_provision_http_get_cert_path();


#endif
