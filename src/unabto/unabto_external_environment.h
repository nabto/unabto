/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_EXTERNAL_ENVIRONMENT_H_
#define _UNABTO_EXTERNAL_ENVIRONMENT_H_

#if NABTO_SLIM
#include <unabto_platform_types.h>
#else
#include <unabto/unabto_env_base.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/********** Platform Random ***************************************************/
/** 
 * Fill buffer with random content.
 * @param buf  the buffer
 * @param len  the length of the buffer
 */
void nabto_random(uint8_t* buf, size_t len);

/*********** UDP init/close ****************************************************/
/**
 * Initialise a udp socket.  This function is called for every socket
 * uNabto creates, this will normally occur two times. One for local
 * connections and one for remote connections.
 *
 * @param localAddr    The local address to bind to.
 * @param localPort    The local port to bind to.
 *                     A port number of 0 gives a random port. The
 *                     pointer may not be NULL.
 * @param socket       To return the created socket descriptor.
 * @return             true iff successfull
 */
bool nabto_init_socket(uint32_t localAddr, uint16_t* localPort, nabto_socket_t* socket);

/**
 * Close a socket. 
 * Close can be called on already closed sockets. And should tolerate this behavior.
 *
 * @param socket the socket to be closed
 */
void nabto_close_socket(nabto_socket_t* socket);

/***************** UDP Read/Write *********************************************/
/**
 * Read message from network (non-blocking).
 * Memory management is handled by the callee.
 *
 * @param socket  the UDP socket
 * @param buf     destination of the received bytes
 * @param len     length of destination buffer
 * @param addr    the senders IP address (host byte order)
 * @param port    the senders UDP port (host byte order)
 * @return        the number of bytes received
 */
ssize_t nabto_read(nabto_socket_t socket,
                   uint8_t*       buf,
                   size_t         len,
                   uint32_t*      addr,
                   uint16_t*      port);

/**
 * Write message to network (blocking) The memory allocation and
 * deallocation for the buffer is handled by the callee.
 *
 * @param socket  the UDP socket
 * @param buf   the bytes to be sent
 * @param len   number of bytes to be sent
 * @param addr  the receivers IP address (host byte order)
 * @param port  the receivers UDP port (host byte order)
 * @return      true when success
 */
ssize_t nabto_write(nabto_socket_t socket,
                    const uint8_t* buf,
                    size_t         len,
                    uint32_t       addr,
                    uint16_t       port);

/**
 * Get the local ip address
 * This function is optional but is used if the
 * NABTO_ENABLE_GET_LOCAL_IP macro is defined to 1.
 * @param ip      ip in host byte order.
 * @return true   iff ip is set to the local ip address.
 */
bool nabto_get_local_ip(uint32_t* ip);



#if NABTO_SET_TIME_FROM_ALIVE
/**
 * Set the time received from the GSP.
 * @param stamp  the unix UTC time (since epoch).
 */
void setTimeFromGSP(uint32_t stamp);
#endif

/*************** DNS related functions ***************************************/

typedef enum {
    NABTO_DNS_OK,
    NABTO_DNS_NOT_FINISHED,
    NABTO_DNS_ERROR
} nabto_dns_status_t;

void nabto_dns_resolver(void);

/**
 * start resolving an ip address 
 * afterwards nabto_resolve_dns will be called until the address is resolved
 */
void nabto_dns_resolve(const char* id);

/**
 * resolve an ipv4 dns address
 * if resolving fails in first attempt we call the function later to 
 * see if the address is resolved. The id is always constant for a device
 * meaning the address could be hardcoded but then devices will fail if 
 * the basestation gets a new ip address.
 *
 * The array of returned ipv4 addresses has a min size of 1 and a max
 * size of NABTO_DNS_RESOLVED_IPS_MAX, if less than
 * NABTO_DNS_RESOLVED_IPS_MAX ips are discovered then the first
 * entries in the result array should be filled first with ips.
 *
 * @param id      name controller hostname
 * @param v4addr  pointer to output ipaddresses array
 * @return false if address is not resolved yet
 */
nabto_dns_status_t nabto_dns_is_resolved(const char* id, uint32_t* v4addr);

/*************** Time stamp related functions ********************************/
/**
 * Has stamp been passed?
 * @param stamp  pointer to the time stamp
 * @return       true if stamp has been passed or is equal
 */
bool nabtoIsStampPassed(nabto_stamp_t *stamp);

/**
 * Get Current time stamp
 * @return current time stamp
 */
nabto_stamp_t nabtoGetStamp(void);

/**
 * Calculate the difference between two time stamps.
 * @param stamp  pointer to the newest time stamp
 * @param stamp  pointer to the oldest time stamp
 * @return       difference between newest and oldest (newest - oldest)
 */
nabto_stamp_diff_t nabtoStampDiff(nabto_stamp_t * newest, nabto_stamp_t *oldest);

/**
 * Convert the time difference to milliseconds
 * @param stamp  the time difference
 * @return       the time in milliseconds
 */
int nabtoStampDiff2ms(nabto_stamp_diff_t diff);


/** Nabto Attachment state. */
typedef enum {
    NABTO_AS_IDLE,      /**< initial state                                          */
    NABTO_AS_WAIT_DNS,  /**< waiting for dns resolving                              */
    NABTO_AS_WAIT_BS,   /**< UINVITE request sent to Base Station(BS)               */
    NABTO_AS_WAIT_DNS_FALLBACK_OPEN_SOCKET, /**< try to make a dns fallback socket              */
    NABTO_AS_WAIT_DNS_FALLBACK_BS, /**< try to send an invite to the basestation via dns fallback */
    NABTO_AS_WAIT_DNS_FALLBACK_UDP_BS, /**< try to send an invite to the basestation via udp after the dns fallback invite has succeded. */
    NABTO_AS_WAIT_GSP,  /**< UINVITE request sent to GSP                            */
    NABTO_AS_ATTACHED   /**< UATTACH request received from and response sent to GSP */
} nabto_state;

#if NABTO_ENABLE_STATUS_CALLBACKS

/**
 * Inform the application of a state change in the uNabto state machine.
 * @param state  the new state.
 */
void unabto_attach_state_changed(nabto_state state);

#endif


#ifdef __cplusplus
} //extern "C"
#endif

#endif
