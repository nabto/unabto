#include "unabto_tunnel_echo.h"

static void unabto_tunnel_echo_closing(tunnel* tunnel, tunnel_event_source event_source);

void unabto_tunnel_echo_parse_command(tunnel* tunnel, tunnel_event_source event_source)
{
    tunnel->tunnelType = TUNNEL_TYPE_ECHO;
    tunnel_send_init_message(tunnel, "+\n");
    tunnel->state = TS_FORWARD;
}

void unabto_tunnel_echo_event(tunnel* tunnel, tunnel_event_source event_source)
{
    if (tunnel->state == TS_FORWARD) {
        unabto_tunnel_echo_forward(tunnel);
    }

    if (tunnel->state == TS_CLOSING) {
        unabto_tunnel_echo_closing(tunnel, event_source);
    }
}

void unabto_tunnel_echo_forward(tunnel* tunnel)
{
    const uint8_t* buf;
    unabto_stream_hint hint;
    size_t readLength = unabto_stream_read(tunnel->stream, &buf, &hint);

    if (readLength > 0) {
        size_t writeLength = unabto_stream_write(tunnel->stream, buf, readLength, &hint);
        if (writeLength > 0) {
            if (!unabto_stream_ack(tunnel->stream, buf, writeLength, &hint)) {
                tunnel->state = TS_CLOSING;
            }
        } else {
            if (hint != UNABTO_STREAM_HINT_OK) {
                tunnel->state = TS_CLOSING;
            }
        }
    } else {
        if (hint !=  UNABTO_STREAM_HINT_OK) {
            tunnel->state = TS_CLOSING;
        }
    }
}

void unabto_tunnel_echo_closing(tunnel* tunnel, tunnel_event_source event_source)
{
    const uint8_t* buf;
    unabto_stream_hint hint;
    size_t readen;

    NABTO_LOG_TRACE(("TS_CLOSING"));

    do {
        readen = unabto_stream_read(tunnel->stream, &buf, &hint);
        if (readen > 0) {
            unabto_stream_ack(tunnel->stream, buf, readen, &hint);
        }
    } while (readen > 0);

    if (unabto_stream_close(tunnel->stream)) {
        unabto_stream_stats info;
        unabto_stream_get_stats(tunnel->stream, &info);

        NABTO_LOG_TRACE(("Closed tunnel successfully"));
        NABTO_LOG_INFO(("Tunnel(%i) closed, " UNABTO_STREAM_STATS_PRI, tunnel->tunnelId, UNABTO_STREAM_STATS_MAKE_PRINTABLE(info)));
        unabto_stream_release(tunnel->stream);
        unabto_tunnel_reset_tunnel_struct(tunnel);
    }
}
