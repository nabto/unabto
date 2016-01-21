#ifndef _UNABTO_PROVISION_HTTP_H
#define _UNABTO_PROVISION_HTTP_H

/**
 * Functions for performing HTTP based provisioning. May be used for
 * finer grained control than what is provided through
 * unabto_provision.h.
 */

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
    UPS_PROV_INVALID_APIKEY
} unabto_provision_status_t;

#define SERVICE_URL_MAX_LENGTH 1024
#define SERVICE_TOKEN_MAX_LENGTH 40
#define SERVICE_POST_MAX_LENGTH 256

#define NABTO_PROVISION_CREATE_PATH                 "/api/1/device/provision"
#define NABTO_PROVISION_VALIDATE_PATH               "/api/1/device/validate"

/**
 * Invoke provisioning webservice as specified in context. Upon
 * success, sets key output parameter (does not insert into nms
 * struct).
 * @returns UPS_OK if provisioning was ok and crypto key was set.
 * @returns UPS_PROV_ALREADY_PROVISIONED if device was provisioned earlier (an existing key can be validated with unabto_provision_validate_key)
 * @returns UPS_PROV_INVALID_KEY if crypto key specified for validation was invalid
 * @returns UPS_PROV_INVALID_APIKEY if api key was not valid
 * @returns UPS_PROV_INVALID_TOKEN if token (activation code) was not valid 
 * @returns UPS_PROV_SERVICE_PROBLEM if service returned an unexpected response 
 * @returns UPS_HTTP_MALFORMED_URL if bad provision hostname specified in context
 * @returns UPS_HTTP_CONNECTION_REFUSED if provision service could not be reached
 * @returns UPS_HTTP_BAD_CACERTFILE if no or an invalid https root cert file was specified
 * @returns UPS_HTTP_SSL_PROBLEM if an https specific problem occurred
 * @returns UPS_HTTP_OTHER if another http related error occured
 */
unabto_provision_status_t unabto_provision_http(nabto_main_setup* nms, provision_context_t* context, uint8_t* key);

/**
 * Invoke provisioning webservice as specified in context to validate specified key.
 * @returns UPS_OK if provisioning was ok and crypto key was set.
 * @returns UPS_PROV_ALREADY_PROVISIONED if device was provisioned earlier (an existing key can be validated with unabto_provision_validate_key)
 * @returns UPS_PROV_INVALID_KEY if crypto key specified for validation was invalid
 * @returns UPS_PROV_INVALID_APIKEY if api key was not valid
 * @returns UPS_PROV_INVALID_TOKEN if token (activation code) was not valid 
 * @returns UPS_PROV_SERVICE_PROBLEM if service returned an unexpected response 
 * @returns UPS_HTTP_MALFORMED_URL if bad provision hostname specified in context
 * @returns UPS_HTTP_CONNECTION_REFUSED if provision service could not be reached
 * @returns UPS_HTTP_BAD_CACERTFILE if no or an invalid https root cert file was specified
 * @returns UPS_HTTP_SSL_PROBLEM if an https specific problem occurred
 * @returns UPS_HTTP_OTHER if another http related error occured
 */
unabto_provision_status_t unabto_provision_validate_key(const uint8_t* id, const uint8_t* key, provision_context_t* context);

/**
 * Set SSL root cert path (full path to file bundle, e.g. cacert.pem).
 */
unabto_provision_status_t unabto_provision_http_set_cert_path(const char* path);

/**
 * Get SSL root cert path (full path to file bundle, e.g. cacert.pem).
 */
const char* unabto_provision_http_get_cert_path();

////////////////////////////////////////////////////////////////////////////////

/**
 * General function to post data to specified URL (not provisioning specific).
 * @returns UPS_OK if http request was successfully invoked and http server returned status 200. Caller must free response.
 * @returns UPS_HTTP_COMPLETE_NOT_200 if http request was completed but http server returned a status != 200. Caller must free response.
 * @returns UPS_HTTP_MALFORMED_URL if specified url was not valid
 * @returns UPS_HTTP_CONNECTION_REFUSED if http server rejected TCP connection
 * @returns UPS_HTTP_OTHER if interaction with http server failed for reasons not otherwise described
 */
unabto_provision_status_t unabto_provision_http_post(const char* url, const char* data, uint16_t* http_status, char** body, const char* headers);

/**
 * General function to get data from specified URL (not provisioning specific). 
 * @returns UPS_OK if http request was successfully invoked and http server returned status 200. Caller must free response.
 * @returns UPS_HTTP_COMPLETE_NOT_200 if http request was completed but http server returned a status != 200. Caller must free response.
 * @returns UPS_HTTP_MALFORMED_URL if specified url was not valid
 * @returns UPS_HTTP_CONNECTION_REFUSED if http server rejected TCP connection
 * @returns UPS_HTTP_OTHER if interaction with http server failed for reasons not otherwise described
 */
unabto_provision_status_t unabto_provision_http_get(const char* url, uint16_t* http_status, char** response);

/**
 * Invokes unabto_provision_http_post with appropriate headers set for json data.
 */
unabto_provision_status_t unabto_provision_http_post_json(const char* url, const char* data, uint16_t* http_status, char** body);

/**
 * Extract body http response, always succeeds - returns body if
 * delimiter found, empty string otherwise. Caller frees response.
 */
void unabto_provision_http_extract_body(char** body, char* response);


#endif
