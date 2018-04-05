/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
/**
 * @file
 * Nabto uServer Connections - Interface.
 */

#ifndef _UNABTO_CONNECTION_H_
#define _UNABTO_CONNECTION_H_

#if NABTO_ENABLE_CONNECTIONS

#include <unabto/unabto_env_base.h>
#include <unabto/unabto_context.h>
#include <unabto/unabto_crypto.h>
#include <unabto/unabto_packet_util.h>
#include <unabto/unabto_connection_type.h>
#include <unabto/unabto_message.h>
#include <unabto/unabto_extended_rendezvous.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UTFS_IDLE,
    UTFS_CONNECTING,
    UTFS_CONNECTED,
    UTFS_HANDSHAKE_SENT,
    UTFS_HANDSHAKE_VERIFIED,
    UTFS_READY_FOR_DATA,
    UTFS_CLOSED
} unabto_tcp_fallback_state;

/** Rendesvous states */
typedef enum {
    RS_CONNECTING,
    RS_DONE
} rendezvous_state;
 
typedef enum {
    CS_IDLE,            /* The connection is idle and free. */
    CS_CONNECTING,      /* We are still in the connecting state. */
    CS_CONNECTED,       /* We are connected and in some data phase */
//    CS_UDP,             /* We have a UDP connection, either p2p or direct. */
//    CS_TCP_FALLBACK,    /* We have a TCP fallback connection. */
    CS_CLOSE_REQUESTED  /* Close is requested on the connection. */
} connection_state;

#define CONNECTION_TIMEOUT 17500ul ///< Connection timeous (7*5000/2).

typedef struct {
    uint32_t packetsReceived;
    uint32_t packetsSent;
    uint32_t bytesReceived;
    uint32_t bytesSent;
    nabto_stamp_t connectionStart;
} connectionStats;

typedef struct {
    nabto_stamp_t timeout;
    nabto_stamp_t timestamp; 
    rendezvous_state state;

    /**
     * Extended rendezvous state
     */ 
    bool openManyPorts;
    nabto_stamp_t openManyPortsStamp;
    uint16_t portsOpened;
    uint16_t socketsOpened;

    unabto_extended_rendezvous_port_sequence portSequence;
#if NABTO_ENABLE_EXTENDED_RENDEZVOUS_MULTIPLE_SOCKETS
    bool openManySockets;
    nabto_stamp_t openManySocketsStamp;
    bool socketsHasBeenClosed;
#endif
} nabto_rendezvous_connect_state;


typedef struct {
    enum {
        WAIT_CONNECT = 0,
        WAIT_VERIFY,
        CONNECTED
    } state;
} unabto_connection_psk_handshake;


#define CON_ATTR_DEFAULT        0x00  /**< unrealiable connection with keep alive       */
#define CON_ATTR_NO_KEEP_ALIVE  0x01  /**< no keep alive traffic                        */
#define CON_ATTR_NO_RETRANSMIT  0x80  /**< no retransmissions (reliable connection)     */

/** The Context of a connection */
struct nabto_connect_s {
#if NABTO_ENABLE_EPOLL
    int epollEventType;
#endif
    
    nabto_rendezvous_connect_state rendezvousConnectState;
    connection_state state; /**< The connection state it's used for
                             * determining both if the connection is
                             * free or occupied and when it's in use
                             * the actual status and type of the
                             * connection. */
    nabto_connection_type type;

    /**
     * psk handshake state
     */
    unabto_connection_psk_handshake pskHandshake; 
    
    uint32_t spnsi; /**< Serverpeer connection identifier. For local
                     * connections this identifier is between 100 and
                     * 1000 and assigned by uNabto. If the connection
                     * is mediated by the basestation the NSI comes
                     * from the GSP which is the same NSI as the GSP
                     * uses for its communication with the client in
                     * the connection phase */
    uint32_t cpnsi; /**< The clientpeer nsi which should be used for
                     * this connection */
    uint8_t                   consi[8];     /**< the controller identification (opt.)     */
    uint8_t*                  nsico;        /**< addr of consi, 0 if not used             */
    nabto_stamp_t             stamp;        /**< the time stamp                           */
    uint32_t                  timeOut;      /**< timeout value (keep alive)               */
    uint8_t                   conAttr;      /**< connection attrinutes (CON_ATTR_*)       */
    uint8_t                   cpEqual;      /**< the endpoints of the peer (CP) are equal */
    uint8_t                   noRendezvous; /**< connection relayed through GSP           */
    uint8_t                   cpAsync;      /**< the Client understands async dialogues   */
    uint8_t                   clientNatType;      /**< The client nat type */
    ipxdata                   cp;           /**< the peer endpoints                       */
    nabto_socket_t            socket;       /**< UDP Socket to use                        */
    nabto_endpoint            peer;         /**< the peer endpoint                        */
    
    connectionStats           stats;        /**< connection stats                         */
    bool                      sendConnectStatistics;
    bool                      sendConnectionEndedStatistics;
    bool                      hasFingerprint;
    uint8_t                   fingerprint[NP_TRUNCATED_SHA256_LENGTH_BYTES]; // client fingerprint



#if NABTO_ENABLE_CLIENT_ID
    char                      clientId[NABTO_CLIENT_ID_MAX_SIZE + 1]; /**< the peer id (e-mail from certificate) */
#else
    char                      clientId[1];  /**< the peer id (e-mail from certificate)    */
#endif
    bool                      isLocal;      /**< used to determine if this is a local connection */

#if NABTO_ENABLE_TCP_FALLBACK
    bool hasTcpFallbackCapabilities; /**< This is true if the
                                      * connection has the
                                      * capabilities and information
                                      * which is needed to make a TCP
                                      * fallback connection. */
    nabto_stamp_t tcpFallbackConnectionStamp;
    unabto_tcp_fallback_state tcpFallbackConnectionState;
    uint8_t fbConAttr; /** connection attributes for the fallback connection. */

    nabto_endpoint fallbackHost; /**< The fallback host to connect
                                  * to. */
    uint8_t gatewayId[20]; /**< The Gateway Id is an unique key which
                            * both the client and server uses to
                            * establish a fallback connection. */

    bool relayIsActive; // data has been transmitted on relay, indicating client has chosen this type
#endif

    /*****************************************************************************************/
    /* fields below has specific init, reinit and release methods.                           */

    nabto_crypto_context cryptoctx;         /**< the crypto context                       */
};
typedef struct nabto_connect_s nabto_connect;


nabto_connect* nabto_reserve_connection(void);

/** Initialize the connection array (all values are zero). */
void nabto_init_connections(void);

/** Closes all connections in the connection array. */
void nabto_terminate_connections(void);


/** Request to release the connection has been received. @param con the connection to be released */
void nabto_release_connection_req(nabto_connect* con);


/** Release a connection. @param con the connection to be released */
void nabto_release_connection(nabto_connect* con);

/** Find the connection. @param spnsi  the identifier.  @return the connection (0 if not found). */
nabto_connect* nabto_find_connection(uint32_t spnsi);

nabto_connect* nabto_find_connection_cp_nsi(uint32_t cpnsi);


/** @return the connection index (for logging). @param  con the connection. */
int nabto_connection_index(nabto_connect* con);

/**
 * Update a connection with a verified event on the connection.
 */
void 
nabto_connection_event(nabto_connect* con, message_event* event);

void nabto_connection_client_aborted(nabto_connect* con);

/**
 * Initialise a new connection.
 * @param ctx      the main context
 * @param hdr      the received packet header
 * @param nsi      to return the connection nsi (only nonzero if readable from request)
 * @param ec       to return the errorcode (only used when returning 0)
 * @param isLocal  true if the connection request comes from the local socket.
 * @return         the new connection (0 if failing).
 *
 * When returning a new connection, nabto_start_connecting must be called to initialise rendezvous.
 * When returning 0, a nonzero *nsi may be used to generate a negative response to the GSP.
 */
nabto_connect* nabto_init_connection(nabto_packet_header* hdr, uint32_t* nsi, uint32_t* ec, bool isLocal);

/**
 * initializes a connection such that it can be used afterwards.
 * @param  ctx The communication context.
 * @param  nsi The NSI for the new connection.
 * @return Null if the connection is not created.
 */
nabto_connect* nabto_get_new_connection(uint32_t nsi);

/**
 * Handle the connect event
 * @param event    network event
 * @param hdr      the decoded packet header
 * @return         true iff fully treated
 * One or more packets may be sent
 */
bool nabto_connect_event(message_event* event, nabto_packet_header* hdr);


bool nabto_connect_event_from_gsp(message_event* event, nabto_packet_header* hdr);

/**
 * Treat timeouts in a connection.
 * @param ctx   the main state
 */
void nabto_time_event_connection(void);

/**
 * Verify the encryption for a connection is ok for the desired
 * security level
 * @param nmc  The main context.
 * @param con  The connection
 * @return     true if the crypto strength is ok for the connection.
 */
bool verify_connection_encryption(nabto_connect* con);

/**
 * @return connection type for a connection.
 */
nabto_connection_type get_connection_type(nabto_connect* con);

uint16_t unabto_count_active_connections(void);
    
bool nabto_write_con(nabto_connect* con, uint8_t* buf, size_t len);

#define nabto_connection_has_keep_alive(con) (((con)->conAttr & CON_ATTR_NO_KEEP_ALIVE) == 0)
#define nabto_connection_is_reliable(con)    (((con)->conAttr & CON_ATTR_NO_RETRANSMIT) != 0)

void send_connection_statistics(nabto_connect* con, uint8_t event);
void send_connection_ended_statistics(nabto_connect* con);

uint8_t* insert_connection_info_payload(uint8_t* ptr, uint8_t* end, nabto_connect* con);

#ifdef __cplusplus
} // extern "C"
#endif

#else

struct nabto_connect_s;
#define nabto_connect void

#endif

#endif
