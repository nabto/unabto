#include <unabto/unabto_dns_fallback.h>
#include <unabto/unabto_common_main.h>

enum {
    BUFFERLENGTH = 2048
};

typedef enum {
    TEST_INIT,
    TEST_OPEN_SOCKET,
    TEST_SEND_PACKET,
    TEST_RECV_PACKET,
    TEST_CLOSE_SESSION,
    TEST_ENDED,
    TEST_FAILED
} test_state;

void tick(test_state* state);

int main(int argc, char** argv)
{
    nabto_main_setup* nms = unabto_init_context();
    nms->id = "abc.starterkit.u.nabto.net";

    nms->dnsAddress = 0x7f000001;
    nms->dnsFallbackDomain = "md.nabto.com";
    
    unabto_dns_fallback_init();

    uint8_t buffer[BUFFERLENGTH];

    test_state state = TEST_INIT;
    
    while (true) {

        size_t readen = unabto_dns_fallback_recv_socket(buffer, BUFFERLENGTH);
        if (readen > 0) {
            unabto_dns_fallback_handle_packet(buffer, readen);
        }
        
        unabto_dns_fallback_handle_timeout();
        
        tick(&state);
        if (state == TEST_ENDED) {
            exit(0);
        }

        if (state == TEST_FAILED) {
            exit(1);
        }
    }
    unabto_dns_fallback_close();
}

void tick(test_state* state)
{
    if (*state == TEST_INIT) {
        *state = TEST_OPEN_SOCKET;
    }
    
    if (*state == TEST_OPEN_SOCKET) {
        unabto_dns_fallback_error_code ec = unabto_dns_fallback_create_socket();
        if (ec == UDF_SOCKET_CREATE_FAILED) {
            NABTO_LOG_ERROR(("Could not open dns socket"));
            *state = TEST_FAILED;
        }
        if (ec == UDF_OK) {
            *state = TEST_SEND_PACKET;
        }
    }

    if (*state == TEST_SEND_PACKET) {
        uint8_t test[42];
        memset(test, 42, 42);
        uint32_t host = 0x7f000001;
        uint16_t port = 9999;
        unabto_dns_fallback_send_to(test, 42, host, port);

        *state = TEST_RECV_PACKET;
    }

    if (*state == TEST_RECV_PACKET) {
        uint8_t recvBuffer[BUFFERLENGTH];
        uint32_t from;
        uint16_t port;
        size_t readen = unabto_dns_fallback_recv_from(recvBuffer, BUFFERLENGTH, &from, &port);
        if (readen > 0) {
            *state = TEST_ENDED;
        }
    }
    
} 
