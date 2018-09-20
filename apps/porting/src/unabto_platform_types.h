#include <stdint.h>
#include <stdbool.h>
#include <time.h>

typedef uint64_t nabto_stamp_t;
typedef int64_t nabto_stamp_diff_t;

//typedef int nabto_socket_t;
typedef struct nabto_socket_t nabto_socket_t;

enum nabto_socket_type {
    NABTO_SOCKET_IP_V4,
    NABTO_SOCKET_IP_V6
};

struct nabto_socket_t {
    int sock;
    enum nabto_socket_type type;
};
