#ifdef WIN32
typedef SOCKET unabtoSocket;
#else
typedef int unabtoSocket;
#endif

size_t write(unabtoSocket sock, const void* buf, size_t len);

int uanbto_close_socket(unabtoSocket sockfd)

bool unabto_tcp_util_would_block();

bool unabto_tcp_util_in_progress();

bool unabto_tcp_util_init_local(unabtoSocket sockfd);

bool unabto_tcp_util_init_remote(unabtoSocket sockfd);

bool unabto_tcp_util_handle_connect(nabto_connect* con);

const char * unabto_tcp_util_get_error_str();
