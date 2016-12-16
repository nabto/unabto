#ifndef _UNABTO_SELECT_API_H_
#define _UNABTO_SELECT_API_H_

#include "unabto_select.h"

void unabto_network_select_add_to_read_fd_set(fd_set* readFds, int* maxReadFd);
void unabto_network_select_read_sockets(fd_set* readFds);

#endif
