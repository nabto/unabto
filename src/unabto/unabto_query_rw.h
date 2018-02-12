/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
 
#ifndef _UNABTO_QUERY_BUFFER_H_
#define _UNABTO_QUERY_BUFFER_H_

#include <unabto/unabto_env_base.h>

#include "unabto/unabto_buffers.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * the query buffers
 */



typedef unabto_abuffer unabto_query_request;
typedef unabto_abuffer unabto_query_response;

//#define unabto_query_response unabto_abuffer

#ifndef PLATFORM_MEMCPY
#define PLATFORM_MEMCPY memcpy
#endif


/**************************************************/
/* Prototypes for query request parameter reading */
/**************************************************/

/**
 * read an uint32_t from the query request buffer
 * @param queryRequest  the query request buffer to read from
 * @param resultValue       the resulting uint32_t
 * @return true on successreadabl
 */
bool unabto_query_read_uint32(unabto_query_request* queryRequest, uint32_t *resultValue);

/**
 * read an uint16_t from the query request buffer
 * @param queryRequest  the query request buffer to read from
 * @param resultValue       the resulting uint16_t
 * @return true on success
 */
bool unabto_query_read_uint16(unabto_query_request* queryRequest, uint16_t *resultValue);

/**
 * read an uint8_t from the query request buffer
 * @param queryRequest  the query request buffer to read from
 * @param resultValue       the resulting uint8_t
 * @return true on success
 */
bool unabto_query_read_uint8(unabto_query_request* queryRequest, uint8_t *resultValue);

/**
 * read an int32_t from the query request buffer
 * @param queryRequest  the query request buffer to read from
 * @param resultValue       the resulting uint32_t
 * @return true on success
 */
bool unabto_query_read_int32(unabto_query_request* queryRequest, int32_t *resultValue);

/**
 * read an int16_t from the query request buffer
 * @param queryRequest  the query request buffer to read from
 * @param resultValue       the resulting uint16_t
 * @return true on success
 */
bool unabto_query_read_int16(unabto_query_request* queryRequest, int16_t *resultValue);

/**
 * read an int8_t from the query request buffer
 * @param queryRequest  the query request buffer to read from
 * @param resultValue       the resulting uint8_t
 * @return true on success
 */
bool unabto_query_read_int8(unabto_query_request* queryRequest, int8_t *resultValue);

/**
 * read a list of uint8 from the query request buffer
 * @param queryRequest  the query request buffer to read from
 * @param list     will be set to the start of the first byte of the byte array
 * @param listLength   set to length of the uint8 sequence 
 * @return true if success
 */
bool unabto_query_read_uint8_list(unabto_query_request* queryRequest, uint8_t **list, uint16_t *listLength);

/**
 * read uint8 list into unabto_buffer without copying the data
 */
bool unabto_query_read_uint8_list_to_buffer_nc(unabto_query_request* queryRequest, unabto_buffer* buffer);

/**
 * read the number of list elements
 * @param queryRequest  the query request buffer to read from
 * @param elementCount  set to number of elements in the list 
 * @return true if success
 */
bool unabto_query_read_list_length(unabto_query_request* queryRequest, uint16_t *elementCount);

/**
 * get the size of the request
 */
uint16_t unabto_query_request_size(unabto_query_request* queryRequest);

/**
 * reset the request query read state to the start again
 */
void unabto_query_request_reset(unabto_query_request* queryRequest);

/***************************************************/
/* Prototypes for query response parameter writing */
/***************************************************/

/**
 * Append an uint8_t to the query response buffer
 * @param queryResponse     the query response buffer
 * @param value        the uint8_t
 * @return false if the operation would have resulted in a buffer overflow.
 */
bool unabto_query_write_uint8(unabto_query_response *queryResponse, uint8_t value);

/**
 * Append an uint16_t to the query response buffer
 * @param queryResponse     the query response buffer
 * @param value        the uint16_t
 * @return false if the operation would have resulted in a buffer overflow.
 */
bool unabto_query_write_uint16(unabto_query_response *queryResponse, uint16_t value);

/**
 * Append an uint32_t to the query response buffer
 * @param queryResponse     the query response buffer
 * @param value        the uint32_t
 * @return false if the operation would have resulted in a buffer overflow.
 */
bool unabto_query_write_uint32(unabto_query_response *queryResponse, uint32_t value);

/**
 * Append an int8_t to the query response buffer
 * @param queryResponse     the query response buffer
 * @param value        the int8_t
 * @return false if the operation would have resulted in a buffer overflow.
 */
bool unabto_query_write_int8(unabto_query_response *queryResponse, int8_t value);

/**
 * Append an int16_t to the query response buffer
 * @param queryResponse     the query response buffer
 * @param value        the int16_t
 * @return false if the operation would have resulted in a buffer overflow.
 */
bool unabto_query_write_int16(unabto_query_response *queryResponse, int16_t value);

/**
 * Append an int32_t to the query response buffer
 * @param queryResponse     the query response buffer
 * @param value        the int32_t
 * @return false if the operation would have resulted in a buffer overflow.
 */
bool unabto_query_write_int32(unabto_query_response *queryResponse, int32_t value);

/**
 * Append a byte array to the query response buffer 
 * This is a short cut for creating a list only consisting of uint8 elements.
 * @param queryResponse     the query response buffer
 * @param list  pointer to a byte array
 * @param listLength number of bytes in the byte array
 * @return true on success
 */
bool unabto_query_write_uint8_list(unabto_query_response * queryResponse, uint8_t *list, uint16_t listLength);

/**
 * return the number of bytes still available for writing in the buffer
 * @param queryResponse  the query response buffer
 * @return the number of bytes available for writing 0 if the buffer is full
 */
size_t unabto_query_write_free_bytes(unabto_query_response * queryResponse);

typedef void *unabto_list_ctx; /* ... */


/**
 * Initializes a list 
 * @param queryResponse     the query response buffer
 * @param   listCtx on return initialized to with the internal list context 
 * @return true on success
 */
bool unabto_query_write_list_start(unabto_query_response* queryResponse, unabto_list_ctx *listCtx);

/**
 * Finalizes a list 
 * @param queryResponse     the query response buffer
 * @param   listCtx the list context returned from the write list start function.
 * @param elementCount number of elements written in the list. 
 * @return true on success
 */
bool unabto_query_write_list_end(unabto_query_response* queryResponse, unabto_list_ctx *listCtx, uint16_t elementCount);

/********************************************************************************
 ******** The following functions are primarily for internal usage - TBM ********
 ********************************************************************************/

#define unabto_query_read_init(queryRequest, buffer) unabto_abuffer_init(queryRequest, buffer)


#define unabto_query_write_init(queryResponse, buffer) unabto_abuffer_init(queryResponse, buffer)

/**
 * Initialize a query request
 */
void unabto_query_request_init(unabto_query_request* queryRequest, unabto_buffer* buffer);

/**
 * Initialize a query response
 */
void unabto_query_response_init(unabto_query_response* queryResponse, unabto_buffer* buffer);


/**
 * Ask how many bytes have been used in the response
 */
uint16_t unabto_query_response_used(unabto_query_response* queryResponse);

/* hackers entry */
#define unabto_query_get_read_head(queryResponse) unabto_abuffer_get_head(queryResponse)

#define unabto_query_get_write_head(queryResponse) unabto_abuffer_get_head(queryResponse)

#define unabto_query_get_write_initial_head(queryResponse) UNABTO_ABUFFER_GET_INITIAL_HEAD(queryResponse)

#define unabto_query_get_write_used(queryResponse) unabto_abuffer_get_used(queryResponse)

#define unabto_query_get_write_unused(queryResponse) unabto_abuffer_get_unused(queryResponse)

#define unabto_query_advance_write_head(queryResponse, offset) unabto_abuffer_advance(queryResponse, offset)


#ifdef __cplusplus
} //extern "C"
#endif

#endif
