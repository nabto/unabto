#ifndef _UNABTO_DNS_FALLBACK_SELECT_H_
#define _UNABTO_DNS_FALLBACK_SELECT_H_

void unabto_dns_fallback_select_add_to_read_fd_set(unabto_dns_session* session, fd_set* readFds, int* maxReadFd);

#endif
