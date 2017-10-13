#include <unabto_platform_types.h>
#include <unabto/unabto_common_main.h>
#include <unabto/unabto_app.h>
#include <modules/tunnel/unabto_tunnel.h>
#include <modules/tunnel/unabto_tunnel_select.h>

/**
 * This is a simple example of a custom tunnel implementation.
 */
int main() {
    nabto_main_setup* nms = unabto_init_context();

    // Setup device id.
    nms->id = "deviceid";
    
    // Setup encryption.
    nms->cryptoSuite = CRYPT_W_AES_CBC_HMAC_SHA256;
    nms->secureAttach = true;
    nms->secureData = true;
    // Copy the preshared key into unabto
    // memcpy(nms->presharedKey, key,16);
    
    // Loop forever.
    tunnel_loop_select();
    return 0;
}

bool tunnel_allow_connection(const char* host, int port) {
    return true;
}

application_event_result application_event(application_request* request, unabto_query_request* readBuffer, unabto_query_response* writeBuffer)
{
    return AER_REQ_INV_QUERY_ID;
}

bool application_poll_query(application_request** applicationRequest) {
    return false;
}

application_event_result application_poll(application_request* applicationRequest, unabto_query_response* writeBuffer) {
    return AER_REQ_INV_QUERY_ID;
}

void application_poll_drop(application_request* applicationRequest) {
}

bool allow_client_access(nabto_connect* connection) {
    // see allow_client_access in main.c for example of an implementation that uses the FP ACL module
    return false;
}

bool unabto_tunnel_allow_client_access(nabto_connect* connection) {
    // see allow_client_access in main.c for example of an implementation that uses the FP ACL module
    return false;
}

#if NABTO_ENABLE_TUNNEL_STATUS_CALLBACKS
void unabto_tunnel_status_callback(tunnel_status_event event, tunnel* tunnel) {
}
#endif

