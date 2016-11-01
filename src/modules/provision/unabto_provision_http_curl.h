#ifndef _UNABTO_PROVISION_HTTP_CURL_H
#define _UNABTO_PROVISION_HTTP_CURL_H

/**
 * Wrapper for curl http client library (to be replaced with lighter
 * weight library some day).
 */

#include <curl/curl.h>

#include "unabto_provision_http.h"

typedef void (*curl_option_callback_func)(CURL *curl, void* userData);

typedef struct unabto_curl_options_callback {
    curl_option_callback_func func;
    void* data;
    struct unabto_curl_options_callback* next;
} unabto_curl_options_callback ;

/**
 * Invoke curl. Caller can set callback to modify options passed to curl.
 * @returns UPS_OK if http request was successfully invoked and http server returned status 200. Caller must free response.
 * @returns UPS_HTTP_COMPLETE_NOT_200 if http request was completed but http server returned a status != 200. Caller must free response.
 * @returns UPS_HTTP_MALFORMED_URL if specified url was not valid
 * @returns UPS_HTTP_CONNECTION_REFUSED if http server rejected TCP connection
 * @returns UPS_HTTP_OTHER if interaction with http server failed for reasons not otherwise described
 */
unabto_provision_status_t unabto_provision_http_invoke_curl(const char* url, uint16_t* http_status, char** response, unabto_curl_options_callback* options_cb);

/**
 * Post data using curl.
 * @returns UPS_OK if http request was successfully invoked and http server returned status 200. Caller must free response.
 * @returns UPS_HTTP_COMPLETE_NOT_200 if http request was completed but http server returned a status != 200. Caller must free response.
 * @returns UPS_HTTP_MALFORMED_URL if specified url was not valid
 * @returns UPS_HTTP_CONNECTION_REFUSED if http server rejected TCP connection
 * @returns UPS_HTTP_OTHER if interaction with http server failed for reasons not otherwise described
 */
unabto_provision_status_t unabto_provision_http_post_curl(const char* url, const char* data, uint16_t* http_status, char** body, const char* headers);

////////////////////////////////////////////////////////////////////////////////

struct unabto_curl_post_struct {
    char postData[HTTP_MAX_POST_LENGTH];
    curl_option_callback_func extra_options_cb;
    struct curl_slist* headers;
};

void unabto_curl_post_cb(CURL *curl, void* userData);

#endif
