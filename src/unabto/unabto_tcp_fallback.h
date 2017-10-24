#ifndef _UNABTO_TCP_FALLBACK_H_
#define _UNABTO_TCP_FALLBACK_H_

/**
 * The TCP fallback module defines a way where uNabto can make tcp
 * connections to a fallback node.
 * 
 * If you need to use the tcp fallback you need to implement a tcp
 * interface which can send a full nabto packet or nothing.
 */

/**
 * The life of a TCP handshake packet is as follows.
 *
 * 1. TCP Connect.
 * 2. Send fallback handshake packet.
 * 3. Wait for receiving a handshake packet from the other end.
 * 4. If the handshake packet from the other end can be verified 
 *    the connection is now opened.
 * 5. Close of the connection.
 */

/**
 * Whenever a packet is received on the fall back connection the
 * method unabto_tcp_fallback_message should be called with the length
 * of the packet which is put into nabtoCommunicationBuffer.
 */

/**
 * A packet is sent through 
 */

#include <unabto/unabto_env_base.h>
#include <unabto/unabto_util.h>
#include <unabto/unabto_connection.h>

#if NABTO_ENABLE_CONNECTIONS
#if NABTO_ENABLE_TCP_FALLBACK

#ifdef __cplusplus
extern "C" {
#endif

/* Logging helpers - prefix each log line */
#define PRI_tcp_fb "[%d;%" PRIu32 "] "
#define TCP_FB_ARGS(c) nabto_connection_index(c), (c)->cpnsi

typedef enum {
    UTFE_OK,
    UTFE_CONNECTING,
    UTFE_QUEUE_FULL,
    UTFE_CONNECTION_CLOSED
} unabto_tcp_fallback_error;


/**
 * Functions which need external implementation in a module.
 */

/**
 * Initialize the fallback module
 */
bool unabto_tcp_fallback_module_init();

/**
 * Deinitialize the fallback module
 */
void unabto_tcp_fallback_module_deinit();
 

/**
 * Initialize the fallback structure. unabto_tcp_fallback_connection
 */
bool unabto_tcp_fallback_init(nabto_connect* con);

/**
 * Close and cleanup fallback resources associated with a connection,
 * this is called when the nabto_connection is released. Meaning this
 * should just do a close on the TCP socket. This function should not
 * trigger a call to unabto_tcp_fallback_socket_closed.
 */
bool unabto_tcp_fallback_close(nabto_connect* con);




/**
 * Functions which is implemented in uNabto core
 */

/**
 * Initialize the connect to a fallback host.
 */
bool unabto_tcp_fallback_connect(nabto_connect* con);

/**
 * In the connect phase of a we need to send the handshake packet
 * after the connect is successful.
 */
bool unabto_tcp_fallback_time_event(nabto_connect* con);

/**
 * When a TCP connection is lost this function should be called such
 * that uNabto cna decide what to do. Depending on the internal state
 * the unabto connection could transfer to several other states.
 */
void unabto_tcp_fallback_socket_closed(nabto_connect* con);

/**
 * When a fallback message is received call this function.
 */
void unabto_tcp_fallback_message(uint16_t ilen);

/**
 * @return UTFE_OK,  on success,
 *         UTFE_QUEUE_FULL,  if no more data can be sent.
 *         UTFE_CONNECTION_CLOSED,  if the underlying fallback connection is closed.
 *         
 */
unabto_tcp_fallback_error unabto_tcp_fallback_write(nabto_connect* con, uint8_t* buffer, size_t bufferLength);

/** return true if fallback connection needs keep alive traffic */
bool unabto_tcp_fallback_has_keep_alive(nabto_connect* con);

/** return true if fallback connection is reliable - no packet loss and packet reordering */
bool unabto_tcp_fallback_is_reliable(nabto_connect* con);

bool nabto_fallback_connect_u_event(uint16_t ilen, nabto_packet_header* hdr);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
#endif

#endif
