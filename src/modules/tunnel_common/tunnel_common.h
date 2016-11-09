#ifndef _TUNNEL_COMMON_H_
#define _TUNNEL_COMMON_H_

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


#define MAX_COMMAND_LENGTH 512
#define MAX_DEVICE_NAME_LENGTH 128
#define MAX_HOST_LENGTH 128

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
    TUNNEL_EVENT_SOURCE_UART_WRITE,
    TUNNEL_EVENT_SOURCE_UART_READ,
    TUNNEL_EVENT_SOURCE_TCP_WRITE,
    TUNNEL_EVENT_SOURCE_TCP_READ,
    TUNNEL_EVENT_SOURCE_UNABTO
} tunnel_event_source;

typedef enum {
    TUNNEL_TYPE_TCP,
    TUNNEL_TYPE_UART,
    TUNNEL_TYPE_ECHO
} tunnel_type;

typedef struct uart_tunnel_static_memory {
    char deviceName[MAX_DEVICE_NAME_LENGTH];
} uart_tunnel_static_memory;

typedef struct tcp_tunnel_static_memory {
    char host[MAX_HOST_LENGTH];
    //~ uint8_t tcpReadBuffer[NABTO_MEMORY_STREAM_SEND_SEGMENT_SIZE];
} tcp_tunnel_static_memory;

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



void close_reader(tunnel* tunnel);
void echo_forward(tunnel* tunnel);




#endif // _TUNNEL_COMMON_H_
