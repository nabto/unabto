/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
/**
 * @file
 * Nabto uServer Application Framework Event Handling - Interface.
 *
 * Handles asynchronious and/or synchronious requests from the application.
 */

#ifndef _UNABTO_APP_ADAPTER_H_
#define _UNABTO_APP_ADAPTER_H_

#include <unabto/unabto_env_base.h>

#if NABTO_ENABLE_CONNECTIONS

#include <unabto/unabto_app.h> //declare application_event_result etc
#include <unabto/unabto_packet_util.h>

#ifdef __cplusplus
extern "C" {
#endif

/** An opaque "handle" returned by #framework_event_query that is supposed
 * to be used in successive calls to the framework. */
struct naf_handle_s;
typedef struct naf_handle_s *naf_handle;

/// The results of #framework_event_query
typedef enum {
    NAF_QUERY_QUEUED,
    NAF_QUERY_OUT_OF_RESOURCES,
    NAF_QUERY_NEW
} naf_query;

/// The initialization of the framework event handler to be called once before usage.
void init_application_event_framework(void);

/**
 * Ask for an event handle corresponding to the given client request.
 * This is the first function to call when handling a client request, so
 * you'll get a handle to pass the other event handling functions below.
 * The event handle returned must be released again using the
 * #framework_release_handle() function.
 * @param clientId   the identity of the client
 * @param reqId      the request identifier
 * @param handle     a pointer to a buffer to copy the handle on success
 * @return           one of the naf_query values
 *
 * The pair (clientId, reqId) identifies the request.
 *
 * The return value is NAF_QUERY_NEW if this is the first time the client
 * request is seen (i.e. a new event handle is created). In this case
 * the buffer pointed to by handle will be assigned the new event handle.
 * Note that this handle will always be 0 when using the SYNC model and
 * always non-zero when using the ASYNC model.
 * The return value is NAF_QUERY_QUEUED if the client request is already
 * known (and is pending to be processed). In this case the buffer pointed
 * to by handle is left unctouched.
 * The return value is NAF_QUERY_OUT_OF_RESOURCES if no more memory is
 * available for a new client request. The buffer pointed to by handle is
 * left unchanged.
 */
naf_query framework_event_query(const char* clientId,
                                uint16_t reqId,
                                naf_handle* handle);

/**
 * Release the event handle returned by #framework_event_query().
 * @param   handle returned by framework_event_query
 * 
 * Upon return of this function, the handle is invalid. Setting the handle
 * to 0 will prevent the handle from being misused afterwards.
 */
void framework_release_handle(naf_handle handle);

/**
 * Handle unabto application event.
 * An application event is typically send through a data request in a nabto
 * packet.
 * Before calling this function the supplied buffer must be filled with the
 * unencrypted nabto request except for the nabto packet headers. The
 * header information for the request must be supplied in the hdr argument.
 * Upon return the supplied buffer may contain a response to be sent to the
 * client. See return values below.
 * This function will call the #application_event() function implemented by
 * the user/customer.
 * @param handle    event handle previously returned by the
 *                  #framework_event_query() function.
 * @param iobuf     the buffer for communicating requests and responses. On
 *                  input the caller is responsible of copying the
 *                  application request excluding the headers to the buffer.
 *                  On return the buffer may contain a response.
 * @param size      the size of iobuf in bytes
 * @param ilen      length of input in bytes
 * @param olen      pointer to buffer that will receive the length of output
 * @param con       the connection to the client
 * @param hdr       the packet header
 * @return          the result of the event, see #application_event_result
 *
 * The return value is AER_REQ_RESPONSE_READY if the request has been
 *    processed and a response is ready in iobuf. The number of bytes copied
 *    to iobuf has been written to olen.
 * The return value is AER_REQ_ACCEPTED if the request has been accepted by
 *    this framework. The framework will call #application_poll_query() and
 *    #application_poll() to receive the response. This answer is only possible
 *    in the ASYNC model. The request will be discardede if an application calls
 *    #framework_release_handle() before #application_poll() has been called.
 * All other return values are error codes. In this case an exception
 *    response has been generated and copied to iobuf. The number of bytes
 *    copied to iobuf has been written to olen.
 */
application_event_result framework_event(naf_handle               handle,
                                         uint8_t*                 iobuf,
                                         uint16_t                 size,
                                         uint16_t                 ilen,
                                         uint16_t*                olen,
                                         nabto_connect*           con,
                                         nabto_packet_header*     hdr);

/**
 * Poll for a response to any of the pending requests in the framework
 * queue. If a response is ready, it will be delivered in the supplied
 * buffer.
 * This function will call the #application_poll() and
 * #application_poll_query() functions implemented by the user/customer.
 * @param buf      the buffer including all nabto headers
 * @param size     size of the buffer in bytes
 * @param olen     pointer to buffer that will receive the length of output
 * @param con      pointer to buffer that will receive the connection to
 *                 send the response.
 * @return         true if and only if a response has been produced and
 *                 copied to buf.
 *
 * The buf, olen and con buffers are only modified if this function returns
 * true, that is if false is returned the buffers are left unchanged with
 * the values given by the caller.
 * If this function returns true, the response in buf should be send on the
 * returned connection (in con).
 */
bool framework_event_poll(uint8_t* buf,
                          uint16_t size,
                          uint16_t* olen,
                          nabto_connect** con);

#ifdef __cplusplus
} //extern "C"
#endif

#endif //NABTO_ENABLE_CONNECTIONS

#endif
