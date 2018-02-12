/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_CONNECTION_TYPE_H_
#define _UNABTO_CONNECTION_TYPE_H_

typedef enum {
    NCT_LOCAL = 0, /**< Local Direct connections */
    NCT_REMOTE_P2P = 1, /**< P2P connections both local and remote,
                         * but they have been mediated through the
                         * basestation. */
    NCT_REMOTE_RELAY = 2, /**< Relay connections with goes through the
                           * GSP which then have a TCP relay
                           * connection to the client. */
    NCT_REMOTE_RELAY_MICRO = 3, /**< TCP Relay connections to the
                                 * client through a relay node. */
    NCT_CONNECTING = 4 /**< When the connection types is not unique.
                        * Meaning we are waiting for data on the
                        * connection such that we know what kind of
                        * connection it is. */
} nabto_connection_type;

#endif
