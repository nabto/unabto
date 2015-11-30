#include "test_webserver.h"
#include "mongoose/mongoose.h"
#include <unabto_platform_types.h>

static int ev_handler(struct mg_connection *conn, enum mg_event ev) {
    switch (ev) {
    case MG_POLL: {
        uint8_t data[1024];
        memset(data, 0, 1024);
        mg_send_data(conn, data, 1024);
        return MG_MORE; // It is important to return MG_MORE after mg_send_file!
    }
    case MG_REQUEST: {
        uint8_t data[1024];
        memset(data, 0, 1024);
        mg_send_data(conn, data, 1024);
        return MG_MORE; // It is important to return MG_MORE after mg_send_file!
    }
    case MG_CLOSE: {
        printf("close\n");
    }
    case MG_AUTH: return MG_TRUE;
    default: return MG_FALSE;
    }
}

#ifdef WIN32
DWORD WINAPI test_webserver(void* arg)
#else
void* test_webserver(void* arg)
#endif
{
    char* port = (char*)arg;
    struct mg_server *server = mg_create_server(NULL, ev_handler);
    mg_set_option(server, "listening_port", port);
    
    printf("Starting on port %s\n", mg_get_option(server, "listening_port"));
    for (;;) mg_poll_server(server, 1000);
    mg_destroy_server(&server);
    
    return 0;
}
