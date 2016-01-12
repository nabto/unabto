#include "unabto_provision_http.h"

static const char* sslCaCertPath_;

unabto_provision_http_status_t unabto_provision_http_set_cert_path(const char* path)
{
    sslCaCertPath_ = path;
    return UPHS_OK;
}

const char* unabto_provision_http_get_cert_path() {
    return sslCaCertPath_;
}
