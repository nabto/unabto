#include "unabto_config.h"
#include "unabto_provision_http.h"
#include "unabto_provision_http_curl.h"
#include "unabto_provision_file.h"

static const char* sslCaCertPath_;

static unabto_provision_status_t map_ws_response_to_status(uint16_t http_status, uint8_t* response);

unabto_provision_status_t unabto_provision_http_set_cert_path(const char* path)
{
    sslCaCertPath_ = path;
    return UPS_OK;
}

const char* unabto_provision_http_get_cert_path() {
    return sslCaCertPath_;
}

unabto_provision_status_t unabto_provision_http_post(const char* url, const char* data, uint16_t* http_status, char** body, const char* headers)
{
    return unabto_provision_http_post_curl(url, data, http_status, body, headers);
}

unabto_provision_status_t unabto_provision_http_post_json(const char* url, const char* data, uint16_t* http_status, char** body)
{
    return unabto_provision_http_post(url, data, http_status, body, "Content-Type: application/json");
}

struct FetchStruct {
    const char* url;
    const char* cookie;
};

void curl_fetch_cb(CURL *curl, void* userData) {
    struct FetchStruct* data = (struct FetchStruct*)userData;
    curl_easy_setopt(curl, CURLOPT_URL, data->url);
    if (data->cookie && strlen(data->cookie) > 0) {
        curl_easy_setopt(curl, CURLOPT_COOKIE, data->cookie);
    }
}

static bool write_provision_url(uint8_t* buffer, size_t len, provision_context_t* context) {
    if (snprintf(buffer, len, "%s://%s%s?APIKEY=%s",
                 context->scheme_, context->host_, NABTO_PROVISION_CREATE_PATH, context->api_key_
            ) >= len) {
        NABTO_LOG_ERROR(("URL buffer too small for request"));
        return false;
    } else {
        return true;
    }
}

static bool write_validate_url(uint8_t* buffer, size_t len, const provision_context_t* context, const uint8_t* id, const uint8_t* key) {
    if (snprintf(buffer, len, "%s://%s%s?APIKEY=%s&id=%s&key=%s",
                 context->scheme_, context->host_, NABTO_PROVISION_VALIDATE_PATH,
                 context->api_key_, id, key
            ) >= len) {
        NABTO_LOG_ERROR(("URL buffer too small for request"));
        return false;
    } else {
        return true;
    }
}

static bool write_provision_json_doc(uint8_t* buffer, size_t len, provision_context_t* context) {
    if (snprintf(buffer, len, "{\"id\": \"%s\", \"token\": \"%s\", \"simple\": 1}", context->id_, context->token_) >= len) {
        NABTO_LOG_ERROR(("JSON buffer too small for request"));
        return false;
    } else {
        return true;
    }
}

static bool validate_string(char delim, char *string)
{
    size_t i, count = 0;
    for (i = 0; i < strlen(string); i++) {
        count += (string[i] == delim);
    }
    return count == 1 && string[i-1] != delim && string[0] != delim;
}

unabto_provision_status_t unabto_provision_http_get(const char* url, uint16_t* http_status, char** response) {
    return unabto_provision_http_invoke_curl(url, http_status, response, NULL, NULL);
}


unabto_provision_status_t unabto_provision_validate_key(const uint8_t* id, const uint8_t* key, provision_context_t* context) {
    char url[SERVICE_URL_MAX_LENGTH];
    if (!write_validate_url(url, sizeof(url), context, id, key)) {
        return UPS_INTERNAL_ERROR;
    }

    uint16_t http_status = 0;
    uint8_t* response;
    unabto_provision_status_t status = unabto_provision_http_get(url, &http_status, (char**)&response);
    if (status == UPS_OK) {
        free(response);
    } else if (status == UPS_HTTP_COMPLETE_NOT_200) {
        status = map_ws_response_to_status(http_status, response);
        free(response);
    }
    return status;
}

void unabto_provision_http_extract_body(char** body, char* response) {
    const char* correct_delimiter = "\r\n\r\n";
    const char* broken_delimiter = "\n\n";
    const char* delimiter;
    delimiter = broken_delimiter; 
    char* bodyWithPrefix = strstr(response, delimiter);
    if (!bodyWithPrefix) {
        delimiter = correct_delimiter;
        bodyWithPrefix = strstr(response, delimiter);
    }
    
    if (bodyWithPrefix) {
        *body = strdup(bodyWithPrefix+strlen(delimiter));
    } else {
        *body = strdup("");
    }
}

static unabto_provision_status_t map_ws_response_to_status(uint16_t http_status, uint8_t* response)
{   
    if (http_status == 403) {
        NABTO_LOG_ERROR(("Bad api key"));
        return UPS_PROV_INVALID_APIKEY;
    }
    if (http_status != 400) {
        NABTO_LOG_ERROR(("Unexpected http status [%d]", http_status));
        return UPS_PROV_SERVICE_PROBLEM;
    }
    
    if (strstr(response, "DEVICE_ALREADY_PROVISIONED") != NULL) {
        // don't error log here - we might have a good key, otherwise we log outside
        NABTO_LOG_TRACE(("Device is already provisioned")); 
        return UPS_PROV_ALREADY_PROVISIONED;
    }
    
    if (strstr(response, "INVALID_CRYPTO_KEY") != NULL) {
        NABTO_LOG_ERROR(("Web service could not validate key it just issued and that was sent back to it for validation"));
        return UPS_PROV_INVALID_KEY;
    }

    if (strstr(response, "TOKEN_DOES_NOT_EXIST") != NULL) {
        NABTO_LOG_ERROR(("Web service could not validate activation code (user specified wrong value or code already used)"));
        return UPS_PROV_INVALID_TOKEN;
    }

    NABTO_LOG_ERROR(("The web service returned error response [%s]", response));
    return UPS_PROV_SERVICE_PROBLEM;
}

static unabto_provision_status_t invoke_provision_service(const uint8_t* url, const uint8_t* input, uint8_t** response) {
    uint16_t http_status = 0;
    unabto_provision_status_t status = unabto_provision_http_post_json(url, input, &http_status, (char**)response);
    if (status == UPS_OK) {
        return UPS_OK;
    } else {
        if (status == UPS_HTTP_COMPLETE_NOT_200) {
            status = map_ws_response_to_status(http_status, *response);
            free(*response);
        }        
        return status;
    }
}

unabto_provision_status_t unabto_provision_http(nabto_main_setup* nms, provision_context_t* context, uint8_t* key)
{
    char url[SERVICE_URL_MAX_LENGTH];
    if (!write_provision_url(url, sizeof(url), context)) {
        return UPS_INTERNAL_ERROR;
    }

    uint8_t json[SERVICE_POST_MAX_LENGTH];
    if (!write_provision_json_doc(json, sizeof(json), context)) {
        return UPS_INTERNAL_ERROR;
    }

    uint8_t* response;

    unabto_provision_status_t status = invoke_provision_service(url, json, &response);

    if (status != UPS_OK) {
        return status;
    }

    // parse response
    if (unabto_provision_parse_data(nms, (char*)response, (char*)(key))) {
        status = UPS_OK;
    } else {
        NABTO_LOG_ERROR(("Invalid web service response: [%s]", response));
        status = UPS_PROV_SERVICE_PROBLEM;
    }
    free(response);
    return status;
}
