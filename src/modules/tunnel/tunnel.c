#include <unabto/unabto_stream.h>
#include <unabto/unabto_app.h>
#include <unabto/unabto_memory.h>
#include <unabto/unabto_common_main.h>
#include <unabto/unabto_main_contexts.h>
#include <unabto/unabto_external_environment.h>
#include <modules/network/epoll/unabto_epoll.h>
#include <modules/tcp_fallback/tcp_fallback_select.h>

#include <string.h>

#include "tunnel.h"

bool init_tunnel_module()
{
    return unabto_tunnel_init_tunnels();
}

void deinit_tunnel_module()
{
    unabto_tunnel_deinit_tunnels();
}

void unabto_stream_accept(unabto_stream* stream) {
    return unabto_tunnel_stream_accept(stream);
}

void unabto_stream_event(unabto_stream* stream, unabto_stream_event_type event) {
    tunnel* t = unabto_tunnel_get_tunnel(stream);
    NABTO_LOG_TRACE(("Stream %i, %i", unabto_stream_index(stream), event));
    tunnel_event(t, TUNNEL_EVENT_SOURCE_UNABTO);
}

void tunnel_event(tunnel* tunnel, tunnel_event_source event_source) {

    NABTO_LOG_TRACE(("Tunnel event on tunnel %i", tunnel));
    if (tunnel->state == TS_IDLE) {
        unabto_tunnel_idle(tunnel, event_source);
        return;
    }

    if (tunnel->state == TS_READ_COMMAND) {
        unabto_tunnel_read_command(tunnel, event_source);
    }

    if (tunnel->state == TS_PARSE_COMMAND) {
        unabto_tunnel_parse_command(tunnel, event_source);
    }

    if (tunnel->state == TS_FAILED_COMMAND) {
        unabto_tunnel_failed_command(tunnel, event_source);
    }
    
    if (tunnel->state >= TS_PARSE_COMMAND) {
        unabto_tunnel_event(tunnel, event_source);
    }
}
