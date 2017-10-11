



write(s,d,l)
closesocket(s)

bool would_block();

bool in_progress();

bool unabto_tcp_util_init_local(tcp_fallback_socket sockfd);

bool unabto_tcp_util_init_remote(tcp_fallback_socket sockfd);

bool unabto_tcp_util_handle_connect(nabto_connect* con);

const char * unabto_tcp_util_get_error_str();
