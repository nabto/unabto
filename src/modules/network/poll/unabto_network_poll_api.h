#ifndef _UNABTO_NETWORK_POLL_API_H_
#define _UNABTO_NETWORK_POLL_API_H_

struct pollfd* unabto_network_poll_add_to_set(struct pollfd* begin, struct pollfd* end);
void unabto_network_poll_read_sockets(struct pollfd* begin, struct pollfd* end);

#endif
