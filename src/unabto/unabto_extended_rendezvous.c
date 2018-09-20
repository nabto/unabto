#include <unabto/unabto_memory.h>
#include "unabto_extended_rendezvous.h"

#include <unabto/unabto_external_environment.h>

void unabto_extended_rendezvous_init_port_sequence(unabto_extended_rendezvous_port_sequence* context, uint16_t startGlobalPort) {
    context->startGlobalPort = startGlobalPort;
    context->lastPortNumber = 4567; // generates a sequence of length 16384
}

uint16_t unabto_extended_rendezvous_next_port(unabto_extended_rendezvous_port_sequence* context, uint16_t count)
{
    /**
     * Since many symnats are portpredictable we start by trying 100 ports
     * around the known external port number.
     * 
     * After that we use ports from a specific series. It's always the
     * same series such that the load on the routers NAT tables will be
     * lighter.
     * 
     * Empirical data shows that devices are never using ports below 1024.
     */
    if (count < 100 &&
        count % 2 == 0)
    {
        uint16_t portNumber = context->startGlobalPort + (count/2) + 1;
        if (portNumber >= 1024) {
            return portNumber;
        }
    } else if (count < 100 &&
               count % 2 == 1)
    {
        uint16_t portNumber = context->startGlobalPort - (count/2) - 1;
        if (portNumber >= 1024) {
            return portNumber;
        }
    }

    do {
        context->lastPortNumber = context->lastPortNumber * 13;
    } while(context->lastPortNumber < 1024);

    return context->lastPortNumber;
}

#if NABTO_ENABLE_EXTENDED_RENDEZVOUS_MULTIPLE_SOCKETS

nabto_socket_t extended_rendezvous_sockets[NABTO_EXTENDED_RENDEZVOUS_MAX_SOCKETS];

void unabto_extended_rendezvous_init() {
    if (nmc.nabtoMainSetup.enableExtendedRendezvousMultipleSockets) {
        int i;
        for (i = 0; i < NABTO_EXTENDED_RENDEZVOUS_MAX_SOCKETS; i++) {
            uint16_t localport = 0;
            nabto_set_invalid_socket(&extended_rendezvous_sockets[i]);
            if (!nabto_init_socket(&localport, &extended_rendezvous_sockets[i])) {
                NABTO_LOG_ERROR(("Cannot initialize extended rendezvous socket %i", i));
            }
        }
    }
}

void unabto_extended_rendezvous_close() {
    if (nmc.nabtoMainSetup.enableExtendedRendezvousMultipleSockets) {
        int i;
        for (i = 0; i < NABTO_EXTENDED_RENDEZVOUS_MAX_SOCKETS; i++) {
            nabto_close_socket(&extended_rendezvous_sockets[i]);
        }
    }
}

#endif
