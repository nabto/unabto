#include "unabto_provision_http.h"

static const char* sslCaCertPath_;

unabto_provision_status_t unabto_provision_http_set_cert_path(const char* path)
{
    sslCaCertPath_ = path;
    return UPS_OK;
}

const char* unabto_provision_http_get_cert_path() {
    return sslCaCertPath_;
}

unabto_provision_status_t unabto_provision_http_post_json(const char* url, const char* data, uint16_t* http_status, char** body) {
    return unabto_provision_http_post(url, data, http_status, body, "Content-Type: application/json");
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

static bool write_validate_url(uint8_t* buffer, size_t len, provision_context_t* context, uint8_t* id, uint8_t* key) {
    if (snprintf(buffer, len, "%s://%s%s?APIKEY=%s&id=%s&key=%s",
                 context->scheme_, context->host_, NABTO_PROVISION_VALIDATE_PATH,
                 context->api_key_, id, key)
            ) >= len) {
        NABTO_LOG_ERROR(("URL buffer too small for request"));
        return false;
    } else {
        return true;
    }
}

static bool write_provision_json_doc(uint8_t* buffer, size_t len, const uint8_t* id, provision_context_t* context) {
    if (snprintf(buffer, len, "{\"id\": \"%s\", \"token\": \"%s\", \"simple\": 1}", id, context->token_) >= len) {
        NABTO_LOG_ERROR("JSON buffer too small for request");
        return false;
    } else {
        return true;
    }
}

bool parse_provision_response(nabto_main_setup *nms, char *response, char *key)
{
    NABTO_LOG_TRACE(("Parsing provision response: [%s]", response));
    char *tok, *delim = ":";
    if (!validate_string(*delim, response)) {
        NABTO_LOG_ERROR(("Invalid provision string: %s", response));
        return false;
    }

    tok = strtok(response, delim);
    if (nms->id == NULL || strcmp(nms->id, tok) != 0) {
        NABTO_LOG_ERROR(("Invalid id in provision string: [id=%s], [tok=%s]", nms->id, tok));
        return false;
    }

    tok = strtok(NULL, delim);
    if (strlen(tok) != PRE_SHARED_KEY_SIZE * 2) {
        NABTO_LOG_ERROR(("Invalid key in provision string: %s", tok));
        return false;
    }

    strcpy(key, tok);
    return true;
}

unabto_provision_status_t unabto_provision_validate_key(const uint8_t* id, const uint8_t* key, provision_context_t* context) {
    char url[SERVICE_URL_MAX_LENGTH];
    if (!write_validate_url(url, sizeof(url), context, id, key)) {
        return UPS_INTERNAL_ERROR;
    }

    uint16_t http_status = 0;
    uint8_t* response;
    unabto_provision_status_t status = http_fetch_absolute_url(url, "", &http_status, (char**)&response);
    if (status == UPS_OK) {
        free(response);
    } else if (status == UPS_HTTP_COMPLETE_NOT_200) {
        status = map_ws_response_to_status(http_status, response);
        free(response);
    }
    return status;
}
static unabto_provision_status_t map_ws_response_to_status(uint16_t http_status, uint8_t* response)
{   
    if (http_status == 403) {
        NABTO_LOG_ERROR(("Bad api key, we assume this is a transient error in the central service"));
        return UPS_PROV_SERVICE_PROBLEM;
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
        return UPS_PROV_INVALID_CODE_ENTERED;
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
            status = map_ws_response_to_status(http_status, response);
            free(*response);
        }        
        return status;
    }
}


// for docs:
//    may return UPS_PROV_ALREADY_PROVISIONED, validate existing key with unabto_provision_validate_key
//    UPS_OK: key set
unabto_provision_status_t unabto_provision(nabto_main_setup* nms, provision_context_t* context, uint8_t* key)
{
    char url[SERVICE_URL_MAX_LENGTH];
    if (!write_provision_url(url, sizeof(url), context)) {
        return UPS_INTERNAL_ERROR;
    }

    uint8_t json[SERVICE_POST_MAX_LENGTH];
    if (!write_provision_json_doc(json, sizeof(json), nms->id, context)) {
        return UPS_INTERNAL_ERROR;
    }

    uint8_t* response;

    unabto_provision_status_t status = invoke_provision_service(url, json, &response);

    if (status != UPS_OK) {
        return status;
    }

    // parse response
    if (parse_provision_response(nms, (char*)response, (char*)(key))) {
        status = UPS_OK;
    } else {
        NABTO_LOG_ERROR(("Invalid web service response: [%s]", response));
        status = UPS_PROV_SERVICE_PROBLEM;
    }
    free(response);
    return status;
}
