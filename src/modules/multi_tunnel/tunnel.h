#ifndef _TUNNEL_H_
#define _TUNNEL_H_

#include <unabto_platform_types.h>
#include <unabto/unabto_stream.h>
#include <unabto/unabto_memory.h>


#define UART_COMMAND "uart"
#define TCP_COMMAND "tunnel"
#define ECHO_COMMAND "echo"

#define MAX_COMMAND_LENGTH 512
#define MAX_DEVICE_NAME_LENGTH 128
#define MAX_HOST_LENGTH 128

#define INVALID_SOCKET -1
typedef socklen_t optlen;
typedef int tunnelSocket;

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
    TUNNEL_TYPE_TCP,
    TUNNEL_TYPE_UART,
    TUNNEL_TYPE_ECHO
} tunnel_type;

typedef enum {
    FS_READ,
    FS_WRITE,
    FS_CLOSING
} forwardState;

typedef enum {
    TUNNEL_EVENT_SOURCE_UART_WRITE,
    TUNNEL_EVENT_SOURCE_UART_READ,
    TUNNEL_EVENT_SOURCE_TCP_WRITE,
    TUNNEL_EVENT_SOURCE_TCP_READ,
    TUNNEL_EVENT_SOURCE_UNABTO
} tunnel_event_source;

typedef struct uart_tunnel_static_memory {
    char deviceName[MAX_DEVICE_NAME_LENGTH];
} uart_tunnel_static_memory;

typedef struct tcp_tunnel_static_memory {
    char host[MAX_HOST_LENGTH];
    //~ uint8_t tcpReadBuffer[NABTO_MEMORY_STREAM_SEND_SEGMENT_SIZE];
} tcp_tunnel_static_memory;
//~ struct tcp_tunnel_static_memory;
//~ struct uart_tunnel_static_memory;

union tunnel_static_memory_union{
    struct tcp_tunnel_static_memory tcp_sm;
    struct uart_tunnel_static_memory uart_sm;
};

typedef struct tunnel_static_memory{
    uint8_t command[MAX_COMMAND_LENGTH];
    union tunnel_static_memory_union stmu;
} tunnel_static_memory;


typedef struct uart_vars{
    int fd;
} uart_vars;

typedef struct tcp_vars{
    int port;
    tunnelSocket sock;
} tcp_vars;

typedef struct echo_vars{
    // TODO: define echo variables
    
} echo_vars;


typedef struct tunnel {
#if NABTO_ENABLE_EPOLL
    int epollEventType;
#endif
    unabto_stream* stream;
    tunnelStates state;
    int commandLength;
    forwardState extReadState;
    forwardState unabtoReadState;
    union {
        uart_vars uart;
        tcp_vars tcp;
        echo_vars echo;
    } tunnel_type_vars;
    
    tunnel_type tunnelType;
    int tunnelId;
    tunnel_static_memory* staticMemory;
} tunnel;

extern NABTO_THREAD_LOCAL_STORAGE tunnel* tunnels;

/* Shared functions */
bool init_tunnel_module();
void deinit_tunnel_module();

void reset_tunnel_struct(tunnel* t);
void reset_unknown_tunnel_struct(tunnel* t);

void tunnel_event(tunnel* state, tunnel_event_source event_source);

/* Uart tunnel specific functions */
void uart_tunnel_set_default_device(const char* device);
const char* uart_tunnel_get_default_device();


/* TCP tunnel specific functions */
bool tunnel_allow_connection(const char* host, int port);

#define DEFAULT_PORT 22
#define DEFAULT_HOST "127.0.0.1"

void tunnel_set_default_host(const char* host);
void tunnel_set_default_port(int port);

const char* tunnel_get_default_host();
int tunnel_get_default_port();


#endif // _TUNNEL_H_
