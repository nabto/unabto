/*
 * Copyright (C) Nabto - All Rights Reserved.
 */

#ifndef _UNABTO_APP_H_
#define _UNABTO_APP_H_

#include <unabto/unabto_env_base.h>
#include <unabto/unabto_protocol_exceptions.h>
#include <unabto/unabto_connection.h>
#include <unabto/unabto_query_rw.h>
#include <unabto/unabto_types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Check/correct consistency of configuration at compile time:
#if NABTO_APPREQ_QUEUE_SIZE < 1
#warning The number of request ressources (NABTO_APPREQ_QUEUE_SIZE) should be greater than 0.
#endif

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

#if NABTO_ENABLE_LOCAL_PSK_CONNECTION
/**
 * Local Pre Shared Key(PSK) connections are local connections
 * protected by a PSK. Whenever such a connection request arrives the
 * following function is called to get a PSK for the connection. If no
 * PSK exists the connection cannot be made. The PSK is a 128 bit
 * secret shared between the device and the client. The shared secret
 * has to be shared in some way out of scope of this functionality.
 *
 * The clientId and fingerprint is available as a way to help the
 * device finding the PSK.
 *
 * @param keyId        key id, always non null, and always has a value
 * @param clientId     id of the client NULL if not provided
 * @param fingerprint  fingerprint of the client certificate, the fingerprint is 
 *                     not validated in local psk connections. Non null, but the value is optional.
 * @param key          output buffer where the key is copied.
 * @return  true iff a psk was found and copied to the psk buffer.
 */
bool unabto_local_psk_connection_get_key(const struct unabto_psk_id* keyId, const char* clientId, const struct unabto_fingerprint* fingerprint, struct unabto_psk* key);
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
 * application_event(), The uNabto application request
 * implementation. This is a function the implementor implements to
 * answer rpc requests. It has two modes, synchronous and
 * asynchronous. If AER_REQ_ACCEPTED is returned an asynchronous
 * request is initiated. All other return values does imply a
 * synchronous request.
 * 
 * @param appreq  the request information
 * @param r_b     buffer, holds the parameters
 * @param w_b     buffer, to retrieve the response message
 * @return        the result
 * - AER_REQ_RESPONSE_READY, the response must be written in the w_b
 * - AER_REQ_INV_QUERY_ID, the queryId is invalid
 * - AER_REQ_ACCEPTED, w_b isn't used. The unabto framework handles the request asynchronously.
 * - the remaining results values are sent to the Client as an exception (w_b is overridden by the caller).
 *
 * WARNING: The read and write buffer use a shared input and output
 * buffer. Before writing to the writeBuffer you need to read all the
 * data you need from the readBuffer.
 *
 * The size of the writeBuffer is limited by the config parameter
 * NABTO_RESPONSE_MAX_SIZE. The maximum request size is limited by the
 * config parameter NABTO_REQUEST_MAX_SIZE.
 *
 * Synchronous requests.
 *
 * A synchronous request is a request where the answer is sent
 * imediately back to the requestor. It is not allowed to block inside
 * the function since it blocks the whole system.
 * 
 * Example:
 * application_event_result application_event(application_request* applicationRequest, unabto_query_request* readBuffer, unabto_query_response* writeBuffer) {
 *     unabto_query_write_int32(writeBuffer, 42);
 *     return AER_REQ_RESPONSE_READY;
 * }
 *
 * Asynchronous requests.
 *
 * To initiate an asynchronous request the function needs to return
 * AER_REQ_ACCEPTED. When the application later has an answer ready to
 * the request, the application should send the opaque application_request pointer
 * back to unabto through the application_poll_query function. Unabto
 * then calls the function application_poll to retrieve the
 * response. After applcation_poll is returned, a call to
 * application_poll_drop is called. The application will need to copy
 * the required request parameters into its own scope. The
 * unabto_query_request pointer is invalid after returning from the
 * function.
 * 
 * Example:
 * static application_request* savedRequest = NULL;
 * application_event_result application_event(application_request* applicationRequest, unabto_query_request* readBuffer, unabto_query_response* writeBuffer) {
 *     if (savedRequest == NULL) {
 *         savedRequest = applicationRequest;
 *         return AER_REQ_ACCEPTED;
 *     } else {
 *         return AER_REQ_OUT_OF_RESOURCES;
 *     }
 * }
 * bool application_poll_query(application_request** applicationRequest) {
 *     if (savedRequest != NULL) { 
 *          *applicationRequest = savedRequest;
 *          return true;
 *     } else {
 *          return false;
 *     }
 * }
 * application_event_result application_poll(application_request* applicationRequest, unabto_query_response* writeBuffer) {
 *     unabto_query_write_int32(writeBuffer, 42);
 *     return AER_REQ_RESPONSE_READY;
 * }
 * void application_poll_drop(application_request* applicationRequest) {
 *     savedRequest = NULL;
 * }
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
 * @param w_b     buffer, to retrieve the response message
 * @return        the result, see #application_event_result
 *
 * Must be called only after #application_poll_query returns true and then with appreq retrieved there.
 *
 * The application must release/delete its internal resource holding
 * the requested request.
 */
application_event_result application_poll(application_request* applicationRequest, unabto_query_response* writeBuffer);

/**
 * Drop the queued request - the framework has discarded it.  This
 * function is called from the framework, the implementor needs to
 * implement this function such that a new query can take the opaque
 * application_request pointer when this function has returned. This
 * function is called exactly once for each time AER_REQ_ACCEPTED is
 * returned from the application_event function.
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
 */
unabto_buffer* get_event_buffer2(size_t maximumSize);
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif
