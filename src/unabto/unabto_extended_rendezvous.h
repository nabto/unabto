#ifndef _UNABTO_EXTENDED_RENDEZVOUS_H_
#define _UNABTO_EXTENDED_RENDEZVOUS_H_

#include <unabto/unabto_env_base.h>


typedef struct {
    uint16_t startGlobalPort;
    uint16_t lastPortNumber;
} unabto_extended_rendezvous_port_sequence;

void unabto_extended_rendezvous_init_port_sequence(unabto_extended_rendezvous_port_sequence* context, uint16_t startGlobalPort);

uint16_t unabto_extended_rendezvous_next_port(unabto_extended_rendezvous_port_sequence* context, uint16_t count);


#if NABTO_ENABLE_EXTENDED_RENDEZVOUS_MULTIPLE_SOCKETS

extern nabto_socket_t extended_rendezvous_sockets[NABTO_EXTENDED_RENDEZVOUS_MAX_SOCKETS];

void unabto_extended_rendezvous_init();

void unabto_extended_rendezvous_close();

#endif



#endif
