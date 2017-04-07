/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
/**
 * @file
 * Deprecated buffer_t, buffer_read_t & buffer_write_t wrapper functions
 */
#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#include <unabto/util/unabto_buffer.h>
#include <unabto/util/unabto_queue.h>
#include <unabto/unabto_util.h>
#include <unabto/unabto_buffers.h>

#include <string.h>

/****************************************/
/* Deprecated buffer read function wrappers */
/****************************************/


bool buffer_read_raw(unabto_abuffer* aBuffer, unabto_buffer* buffer)
{
    uint8_t *data;
    uint16_t len;
    if (NULL == buffer || !unabto_query_read_uint8_list(aBuffer, &data, &len) || len > unabto_buffer_get_size(buffer))
    {
        return false;
    }
    memcpy(buffer->data, (void*)data, len);
    unabto_abuffer_advance(aBuffer, len);
    buffer->size = len;
    return true;
}


bool buffer_read_raw_nc(unabto_abuffer* aBuffer, unabto_buffer* buffer)
{
    uint8_t *data;
    uint16_t len;
    if (NULL == buffer || !unabto_query_read_uint8_list(aBuffer, &data, &len))
    {
        return false;
    }
    unabto_buffer_init(buffer, data, len);
    return true;
}
/*
void buffer_read_reset(buffer_read_t* buffer) {
    buffer->position = 0;
}
*/
/*****************************************/
/* Deprecated buffer write function wrappers */
/*****************************************/

bool buffer_write_raw_from_queue(buffer_write_t* w_buf, queue_t* q) {
    uint16_t count = q->count;
    uint16_t i;
    uint8_t elm;
    unabto_list_ctx listCtx;

    if (unabto_query_get_write_unused(w_buf) < (uint16_t) (sizeof(uint16_t) + count)) {
        return false; // not enough space in destination buffer
    }
    
    unabto_query_write_list_start(w_buf, &listCtx);
    for (i = 0; i < count; i++) {
        queue_dequeue(q, &elm);
        unabto_query_write_uint8(w_buf, elm);
    }
    
    unabto_query_write_list_end(w_buf, &listCtx, count);

    return true;
}

void buffer_read_init(buffer_read_t* read_buf, buffer_t* buf)
{
    read_buf->buffer = buf;
    read_buf->position = 0;
}
