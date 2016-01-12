#include "unabto_provision_http.h"
#include "unabto_provision_http_curl.h"

// logging
#include <unabto/unabto_app.h>

#include <curl/curl.h>

struct MemoryStruct {
  char *memory;
  size_t size;
};

size_t curl_writer_cb(char* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
 
    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if(mem->memory == NULL) {
        /* out of memory! */ 
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }
    
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    
    return realsize;
}

void curl_post_cb(CURL *curl, void* userData) {
    struct PostStruct* data = (struct PostStruct*)userData;
    curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data->postData);
    if (data->extra_options_cb) {
        data->extra_options_cb(curl, userData);
    }
}

void curl_header_cb(CURL *curl, void* userData) {
    struct PostStruct* data = (struct PostStruct*)userData;
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, data->headers);
}

unabto_provision_http_status_t unabto_provision_http_invoke_curl(const char* url, uint16_t* http_status, char** response, curl_option_callback_func options_cb, void* options_data)
{
    NABTO_LOG_TRACE(("Invoking %s", url));

    CURL *curl;       
    curl = curl_easy_init();
    if (!curl) {
        NABTO_LOG_ERROR(("Curl could not be initialized"));
        return UPHS_HTTP_OTHER;
    }

    struct MemoryStruct chunk;
    chunk.memory = malloc(1024);
    if (!chunk.memory) {
        curl_easy_cleanup(curl);
        return UPHS_HTTP_OTHER;
    }
    chunk.size = 0;

#ifdef DEBUG
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_CAINFO, unabto_provision_http_get_cert_path());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writer_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
    if (options_cb) {
        options_cb(curl, options_data);
    }
    CURLcode res = curl_easy_perform(curl);
    unabto_provision_http_status_t status = UPHS_OK;

    if (res == CURLE_OK) {
        chunk.memory = realloc(chunk.memory, chunk.size+1);
        chunk.memory[chunk.size] = 0;
        *response = strdup(chunk.memory);
        long curlHttpStatus = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &curlHttpStatus);
        *http_status = (uint16_t)(curlHttpStatus);
        status = *http_status == 200 ? UPHS_OK : UPHS_HTTP_COMPLETE_NOT_200;

    } else if (res == CURLE_COULDNT_CONNECT) {
        status = UPHS_HTTP_CONNECTION_REFUSED;

    } else if (res == CURLE_URL_MALFORMAT) {
        status = UPHS_HTTP_MALFORMED_URL;

    } else if (res == CURLE_SSL_CACERT_BADFILE) {
        status = UPHS_HTTP_BAD_CACERTFILE;
        NABTO_LOG_ERROR(("Curl perform failed - bad SSL cert file"));

    } else if (res == CURLE_SSL_CACERT ||
               res == CURLE_SSL_CONNECT_ERROR ||
               res == CURLE_SSL_CERTPROBLEM) {
        status = UPHS_HTTP_SSL_PROBLEM;
        NABTO_LOG_ERROR(("Curl perform failed - SSL problem: [%d]", res));
        
    } else {
        NABTO_LOG_ERROR(("Curl perform failed: [%d] \"%s\"", res, curl_easy_strerror(res)));
        status = UPHS_HTTP_OTHER;
    }
    free(chunk.memory);
    curl_easy_cleanup(curl);
    return status;
}

unabto_provision_http_status_t unabto_provision_http_post(const char* url, const char* data, uint16_t* http_status, char** body, const char* headers) {
    struct PostStruct postStruct;
    snprintf(postStruct.postData, sizeof(postStruct.postData), data);
    NABTO_LOG_TRACE(("Posting to url [%s]: [%s]", url, data));
    postStruct.headers = NULL;
    if (strlen(headers) > 0) {
        postStruct.extra_options_cb = curl_header_cb;    
        postStruct.headers = curl_slist_append(postStruct.headers, "Content-Type: application/json");
    } else {
        postStruct.extra_options_cb = NULL;    
    }
    char* response;
    unabto_provision_http_status_t status = unabto_provision_http_invoke_curl(url, http_status, &response, curl_post_cb, &postStruct);
    curl_slist_free_all(postStruct.headers);
    if (status == UPHS_OK || status == UPHS_HTTP_COMPLETE_NOT_200) {
        http_extract_body(body, response);
        free(response);
    }
    NABTO_LOG_INFO(("Posted data to url %s, internal status %d", url, status));
    return status;
}


