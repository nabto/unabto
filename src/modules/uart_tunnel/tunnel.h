#ifndef _TUNNEL_H_
#define _TUNNEL_H_

#include <unabto_platform_types.h>
#include <unabto/unabto_stream.h>

typedef enum {
    TS_IDLE,
    TS_READ_COMMAND,
    TS_PARSE_COMMAND,
    TS_OPEN_UART,
    TS_OPENING_SOCKET,
    TS_FORWARD,
    TS_CLOSING
} tunnelStates;

typedef enum {
    FS_READ,
    FS_WRITE,
    FS_CLOSING
} forwardState;

typedef enum {
    TUNNEL_EVENT_SOURCE_UART_WRITE,
    TUNNEL_EVENT_SOURCE_UART_READ,
    TUNNEL_EVENT_SOURCE_UNABTO
} tunnel_event_source;

struct tunnel_static_memory;

typedef struct tunnel {
#if NABTO_ENABLE_EPOLL
    int epollEventType;
#endif
    unabto_stream* stream;
    tunnelStates state;
    int commandLength;
    int fd;
    forwardState uartReadState;
    forwardState unabtoReadState;
    int tunnelId;
    struct tunnel_static_memory* staticMemory;
} tunnel;

extern NABTO_THREAD_LOCAL_STORAGE tunnel* tunnels;

void uart_tunnel_set_default_device(const char* device);


const char* uart_tunnel_get_default_device();

bool init_tunnel_module();
void deinit_tunnel_module();

void reset_tunnel_struct(tunnel* t);

void tunnel_event(tunnel* state, tunnel_event_source event_source);

#endif // _TUNNEL_H_
