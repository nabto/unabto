
#include <unabto/unabto_stream.h>
#include <unabto/unabto_memory.h>
#include <errno.h>
#include <fcntl.h>
#include <modules/network/epoll/unabto_epoll.h>

#include <modules/tunnel_common/tunnel_common.h>

#if NABTO_ENABLE_EPOLL
#include <sys/epoll.h>
#endif

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


void close_reader(tunnel* tunnel) {
    NABTO_LOG_TRACE(("close reader"));
    unabto_stream_close(tunnel->stream);
    tunnel->extReadState = FS_CLOSING;
}


/**
 * Echo unabto back to unabto
 */
void echo_forward(tunnel* tunnel){
 
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
