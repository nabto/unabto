#ifndef _TUNNEL_H_
#define _TUNNEL_H_

#include <unabto_platform_types.h>
#include <unabto/unabto_stream.h>

#if defined(WIN32) || defined(WINCE)
// use winsock
#define WINSOCK 1
#endif


#if defined(WINSOCK)
#define SHUT_WR SD_BOTH
#define MSG_NOSIGNAL 0
#define _SCL_SECURE_NO_WARNINGS
typedef SOCKET tunnelSocket;
#define close closesocket
typedef int optlen;
#else
typedef int tunnelSocket;
typedef socklen_t optlen;
#define INVALID_SOCKET -1
#endif


typedef enum {
    TS_IDLE,
    TS_READ_COMMAND,
    TS_PARSE_COMMAND,
    TS_OPEN_SOCKET,
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
    TUNNEL_EVENT_SOURCE_TCP_WRITE,
    TUNNEL_EVENT_SOURCE_TCP_READ,
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
    int port;
    tunnelSocket sock;
    forwardState tcpReadState;
    forwardState unabtoReadState;
    int tcpReadBufferSize;
    int tcpReadBufferSent;
    int tunnelId;
    struct tunnel_static_memory* staticMemory;
} tunnel;

extern NABTO_THREAD_LOCAL_STORAGE tunnel tunnels[NABTO_STREAM_MAX_STREAMS];

/**
 * return false to disallow connections to the specified host:port
 */
bool tunnel_allow_connection(const char* host, int port);

#define DEFAULT_PORT 22
#define DEFAULT_HOST "127.0.0.1"

void tunnel_set_default_host(const char* host);
void tunnel_set_default_port(int port);

const char* tunnel_get_default_host();
int tunnel_get_default_port();


void reset_tunnel_struct(tunnel* t);

void tunnel_event(tunnel* state, tunnel_event_source event_source);

#endif // _TUNNEL_H_
