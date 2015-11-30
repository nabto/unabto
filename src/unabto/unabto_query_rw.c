#include "unabto_util.h"
#include "unabto_query_rw.h"


/*****************************************************
 ************** Query read functions ****************
 *****************************************************/

/*
bool unabto_query_read_init(unabto_query_request* queryRequest, uint8_t *data, uint16_t dataSize) 
{
    return unabto_abuffer_init(queryRequest, data, dataSize);
}
*/

uint16_t unabto_query_request_get_size(unabto_query_request* queryRequest)
{
    return unabto_abuffer_get_size(queryRequest);
}

    
#define QUERY_INT_READ_FUNCTION_DEF(sign, bitSize) \
bool unabto_query_read_ ## sign ## int ## bitSize(unabto_query_request* queryRequest, sign ## int ## bitSize ## _t *resultValue) \
{ \
    if (NULL != queryRequest && UNABTO_ABUFFER_GET_UNUSED(queryRequest) < sizeof(uint ## bitSize ## _t)) \
    { \
        return false; \
    } \
    if (NULL != resultValue) \
    { \
        READ_U ## bitSize(*resultValue, UNABTO_ABUFFER_GET_HEAD(queryRequest)); \
    } \
    UNABTO_ABUFFER_ADVANCE(queryRequest, sizeof(uint ## bitSize ## _t)); \
    return true; \
}
    
    
QUERY_INT_READ_FUNCTION_DEF(, 8)

QUERY_INT_READ_FUNCTION_DEF(, 16)

QUERY_INT_READ_FUNCTION_DEF(, 32)

QUERY_INT_READ_FUNCTION_DEF(u, 8)

QUERY_INT_READ_FUNCTION_DEF(u, 16)

QUERY_INT_READ_FUNCTION_DEF(u, 32)   

    

bool unabto_query_read_uint8_list(unabto_query_request* queryRequest, uint8_t **list, uint16_t *listLength)
{
    uint16_t length;
    if (NULL == queryRequest || !unabto_query_read_uint16(queryRequest, &length) || UNABTO_ABUFFER_GET_UNUSED(queryRequest) < length)
    {
        return false;
    }

    if (NULL != list && NULL != listLength)
    {
        *listLength = length;
        *list = UNABTO_ABUFFER_GET_HEAD(queryRequest);
    }
    UNABTO_ABUFFER_ADVANCE(queryRequest, length);
    return true;
}


bool unabto_query_read_list_length(unabto_query_request* queryRequest, uint16_t *listLength)
{
    uint16_t length;
    if (NULL == queryRequest || !unabto_query_read_uint16(queryRequest, &length) || UNABTO_ABUFFER_GET_UNUSED(queryRequest) < length)
    {
        return false;
    }
    
    if (NULL != listLength)
        *listLength = length;
    return true;
}


 
 
/*****************************************************
 ************** Query write functions ****************
 *****************************************************/

/*
bool unabto_query_write_init(unabto_query_response* queryResponse, uint8_t *data, uint16_t dataSize) 
{
    return unabto_abuffer_init(queryResponse, data, dataSize);
}


uint16_t unabto_query_get_write_size(unabto_query_response* queryResponse)
{
    return unabto_abuffer_get_size(queryResponse);
}


uint16_t unabto_query_get_write_unused(unabto_query_request* queryResponse)
{
    return unabto_abuffer_get_unused(queryResponse);
}


uint16_t unabto_query_get_write_used(unabto_query_response *queryResponse)
{
    return unabto_abuffer_get_used(queryResponse);
}


*/



 
#define QUERY_INT_WRITE_FUNCTION_DEF(sign, bitSize) \
bool unabto_query_write_ ## sign ## int ## bitSize(unabto_query_response *queryResponse, sign ## int ## bitSize ## _t value) \
{\
    if (NULL == queryResponse || UNABTO_ABUFFER_GET_UNUSED(queryResponse) < sizeof(uint ## bitSize ## _t)) \
    { \
        return false; \
    } \
    else \
    { \
        WRITE_U ## bitSize(UNABTO_ABUFFER_GET_HEAD(queryResponse), (uint ## bitSize ## _t)value); \
        UNABTO_ABUFFER_ADVANCE(queryResponse, sizeof(uint ## bitSize ## _t)); \
        return true; \
    } \
}    
 
QUERY_INT_WRITE_FUNCTION_DEF(, 8)

QUERY_INT_WRITE_FUNCTION_DEF(, 16)

QUERY_INT_WRITE_FUNCTION_DEF(, 32)

QUERY_INT_WRITE_FUNCTION_DEF(u, 8)

QUERY_INT_WRITE_FUNCTION_DEF(u, 16)

QUERY_INT_WRITE_FUNCTION_DEF(u, 32)   



bool unabto_query_write_uint8_list(unabto_query_response * queryResponse, uint8_t *list, uint16_t listLength)
{
    if (NULL == queryResponse || (NULL == list && listLength > 0) || UNABTO_ABUFFER_GET_UNUSED(queryResponse) < sizeof(uint16_t) + listLength)
    {
        return false;
    } 
    unabto_query_write_uint16(queryResponse, listLength); // won't fail
    return unabto_abuffer_add_data(queryResponse, list, listLength);
}



bool unabto_query_write_list_start(unabto_query_response* queryResponse, unabto_list_ctx *listCtx)
{
    if (NULL == queryResponse || NULL == listCtx || UNABTO_ABUFFER_GET_UNUSED(queryResponse) < sizeof(uint16_t))
    {
        return false;
    }

    *listCtx = (unabto_list_ctx*)UNABTO_ABUFFER_GET_HEAD(queryResponse);
    unabto_query_write_uint16(queryResponse, 0); // won't fail
    return true;
}


bool unabto_query_write_list_end(unabto_query_response* queryResponse, unabto_list_ctx *listCtx, uint16_t elementCount)
{
  /* Check if the listCtx argument is reasonable valid etc. */

    if (NULL == queryResponse
        || NULL == listCtx
        || NULL == *listCtx
        || UNABTO_ABUFFER_GET_INITIAL_HEAD(queryResponse) > (uint8_t*)*listCtx
        || UNABTO_ABUFFER_GET_HEAD(queryResponse) - sizeof(uint16_t) < (uint8_t*)*listCtx)
    {
        return false;
    }
    WRITE_U16(*listCtx, elementCount); 
    return true;
}

/*
bool unabto_query_get_write_head(unabto_query_response * queryResponse, uint8_t **responseHead)
{
    if (NULL == queryResponse)
    {
        return false;
    }
    *responseHead = UNABTO_ABUFFER_GET_HEAD(queryResponse);
    return true;
}
*/
