#include "unabto_dns_fallback_select.h"

void unabto_dns_fallback_select_add_to_read_fd_set(unabto_dns_session* session, fd_set* readFds, int* maxReadFd)
{
    FD_SET(session->socket, readFds);
    *maxReadFd = MAX(*maxReadFd, session->socket);
}
