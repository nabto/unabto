/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
/**
 * @file
 * Nabto uServer test example server specific protocol including <a href="#access_control">Access Control</a>- Interface.
 *
 * ### <a name="access_control">Access Control.</a>
 *
 * All requests to the application holds (a pointer to) the identity of the client
 * having sent the request. The identity of the client is the e-mail-address found
 * in the certificate used to establish the connection.
 * Thus the application may execute any kind of access control before deciding how
 * to respond to the request.
 *
 * Additionally, unabto may implement a connection based access control,
 * i.e. when a client establishes a connection, the application is notified,
 * see #allow_client_access, thus the application is able to deny the connection attempt.
 */

#ifndef _UNABTO_APP_H_
#define _UNABTO_APP_H_

#include <unabto/unabto_env_base.h>
#include <unabto/unabto_protocol_exceptions.h>
#include <unabto/unabto_connection.h>
#include <unabto/unabto_query_rw.h>

#ifdef __cplusplus
extern "C" {
#endif

#if NABTO_APPLICATION_EVENT_MODEL_ASYNC

// Check/correct consistency of configuration at compile time:
#if NABTO_APPREQ_QUEUE_SIZE < 1
#error The number of request ressources, NABTO_APPREQ_QUEUE_SIZE must be positive
#endif

#endif // NABTO_APPLICATION_EVENT_MODEL_ASYNC == 0

/******************************************************************************/

//typedef struct nabto_connect_s nabto_connect;

#if NABTO_ENABLE_CONNECTION_ESTABLISHMENT_ACL_CHECK
/**
 * Query at connection establishment whether a client is allowed access (check ACL, optional functionality).
 * @param connection  the connection being established
 * @return            true if access to the devices is allowed
 */
bool allow_client_access(nabto_connect* connection);
#endif

#if NABTO_ENABLE_TUNNEL_OPEN_ACL_CHECK
/**
 * Query at tunnel open request whether a client is allowed access (check ACL, optional functionality).
 * @param connection  the connection being established
 * @return            true if access to the devices is allowed
 */
bool allow_client_tunnel(nabto_connect* connection);
#endif

    
/** Identifies the request including the caller */
typedef struct {
    uint32_t       queryId;    ///< The query id.
    const char*    clientId;   ///< Null terminated client acl credentials
    nabto_connect* connection; ///< The Connection the query is on 
    bool           isLocal;    ///< True if the request came from a local connection
    bool           isLegacy;   ///< True if the request came from a legacy connection
                               ///< Meaning no support for all connection related features.
} application_request;


/** The results of the application_event function (also used by intermediate functions). */
typedef enum {
    AER_REQ_RESPONSE_READY = 0, ///< the response is ready and written into the supplied buffer
#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
    AER_REQ_ACCEPTED = -1,      ///< the request has been accepted by the application, the response wil be delivered using #application_poll_query() and #application_poll()
#endif

    // The next values are sent to the ClientPeer in exception packets,
    // i.e. the values MUST NOT be changed!
    AER_REQ_NOT_READY = NP_E_NOT_READY,
    AER_REQ_NO_ACCESS = NP_E_NO_ACCESS,
    AER_REQ_TOO_SMALL = NP_E_TOO_SMALL,
    AER_REQ_TOO_LARGE = NP_E_TOO_LARGE,
    AER_REQ_INV_QUERY_ID = NP_E_INV_QUERY_ID,
    AER_REQ_RSP_TOO_LARGE = NP_E_RSP_TOO_LARGE,
    AER_REQ_OUT_OF_RESOURCES = NP_E_OUT_OF_RESOURCES,
    AER_REQ_SYSTEM_ERROR = NP_E_SYSTEM_ERROR,
    AER_REQ_NO_QUERY_ID = NP_E_NO_QUERY_ID
    // End of values sent to a ClientPeer
} application_event_result;


/**
 * The uNabto protocol implementation (application logic)
 * @param appreq  the request information
 * @param r_b     buffer, holds the parameters
 * @param w_b     buffer, to retrieve the response message
 * @return        the result
 * - AER_REQ_RESPONSE_READY, the response must be written in the w_b
 * - AER_REQ_INV_QUERY_ID, the queryId is invalid
 * - AER_REQ_QUEUED, w_b isn't used and the application allocates memory for building the response internally.
 * - the remaining results values are sent to the Client as an exception (w_b is overridden by the caller).
 *
 * The read and write buffer may use a shared buffer so you have to
 * read the input before writing to the output.
 */
application_event_result application_event(application_request* applicationRequest, unabto_query_request* readBuffer, unabto_query_response* writeBuffer);


#if NABTO_APPLICATION_EVENT_MODEL_ASYNC

/**
 * Query whether a response to a queued request is ready.
 * @param appreq  to return the request being ready
 * @return        true if a response is ready
 */
bool application_poll_query(application_request** applicationRequest);


/**
 * Retrieve the response from a queued request.
 * @param appreq  the application request being responded to (from #application_poll_query)
 * @param r_b     buffer, holds the parameters
 * @param w_b     buffer, to retrieve the response message
 * @return        the result, see #application_event_result
 *
 * Must be called only after #application_poll_query returns true and then with appreq retrieved there.
 *
 * The application must release/delete its internal ressouce holding the requested request.
 */
application_event_result application_poll(application_request* applicationRequest, unabto_query_request* readBuffer, unabto_query_response* writeBuffer);

/**
 * Drop the queued request - the framework has discarded it.
 */
void application_poll_drop(application_request* applicationRequest);

#endif // NABTO_APPLICATION_EVENT_MODEL_ASYNC


#if NABTO_ENABLE_EVENTCHANNEL
/**
 * fill an event buffer with piggyback data. The framework calls
 * this function when piggyback data is needed. The buffer returned
 * must stay in memory until the next call to get_event_buffer2.
 * @param maximumSize  the largest message that can be delivered
 * @return             pointer to global piggyback message (or 0)
 *
 * Find example in projects/consulo/src/consulo_application.c
 */
unabto_buffer* get_event_buffer2(size_t maximumSize);
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif
