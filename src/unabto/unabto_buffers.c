/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
/**
 * @file
 * unabto_buffer & unabto_abuffer
 */

#include "unabto_buffers.h"
#include <string.h>

/********************** unabto_buffer ************************/  


void unabto_buffer_init(unabto_buffer *buffer, uint8_t *data, uint16_t size) {
    buffer->size = size;
    buffer->data = data;
}


uint8_t* unabto_buffer_get_data(const unabto_buffer *buffer) {
    return buffer->data;
}


int unabto_buffer_get_size(const unabto_buffer* buffer) {
    if (NULL == buffer)
        return 0;
    else
        return (int) buffer->size;
}


bool unabto_buffer_copy(unabto_buffer* dest, const unabto_buffer* src) {
    if (dest->size < src->size) return false;
    dest->size = src->size;
    memcpy(dest->data, (const void*) src->data, src->size);
    return true;
}


int unabto_buffer_cmp(const unabto_buffer *b1, const unabto_buffer *b2)
{
    if (NULL == b1 && NULL == b2)
        return 0;
    if (NULL == b1) 
        return -1;
     if (NULL == b2) 
        return 1;
    if (unabto_buffer_get_size(b1) != unabto_buffer_get_size(b2))
        return unabto_buffer_get_size(b1) < unabto_buffer_get_size(b2)?-1:1;
    return strncmp((const char*)unabto_buffer_get_data(b1), (const char*)unabto_buffer_get_data(b2), unabto_buffer_get_size(b1));
}


/********************** unabto_abuffer ************************/  

bool unabto_abuffer_reset(unabto_abuffer* aBuffer)
{
    if (NULL == aBuffer)
    {
        return false;
    }
    UNABTO_ABUFFER_RESET(aBuffer);
    return true;
}



bool unabto_abuffer_init(unabto_abuffer* aBuffer, unabto_buffer *buffer)
{
    if (NULL == aBuffer || NULL == buffer)
    {
        return false;
    }
    aBuffer->buffer = buffer;
    UNABTO_ABUFFER_RESET(aBuffer);
    return true;
}


/*
bool unabto_abuffer_init(unabto_abuffer* aBuffer, uint8_t *data, uint16_t size)
{
    if (NULL == aBuffer || NULL == data)
    {
        return false;
    }
    unabto_buffer_init(&aBuffer->buffer, data, size);
    UNABTO_ABUFFER_RESET(aBuffer);
    return true;
}
*/

uint8_t *unabto_abuffer_get_head(const unabto_abuffer* aBuffer)
{
    if (NULL == aBuffer)
    {
        return NULL;
    }
    return UNABTO_ABUFFER_GET_HEAD(aBuffer);
}


uint16_t unabto_abuffer_get_size(const unabto_abuffer* aBuffer)
{
    if (NULL == aBuffer)
    {
        return 0;
    }   
    return UNABTO_ABUFFER_GET_SIZE(aBuffer);
}

uint16_t unabto_abuffer_get_used(const unabto_abuffer* aBuffer)
{
    if (NULL == aBuffer)
    {
        return 0;
    }   
    return UNABTO_ABUFFER_GET_USED(aBuffer);
}


uint16_t unabto_abuffer_get_unused(const unabto_abuffer* aBuffer)
{
    if (NULL == aBuffer)
    {
        return 0;
    }   
    return UNABTO_ABUFFER_GET_UNUSED(aBuffer);
}


bool unabto_abuffer_advance(unabto_abuffer* aBuffer, uint16_t offset)
{
    if (NULL == aBuffer || UNABTO_ABUFFER_GET_UNUSED(aBuffer) < offset)
    {
        return false;
    }
    UNABTO_ABUFFER_ADVANCE(aBuffer, offset);
    return true;
}

bool unabto_abuffer_copy(unabto_abuffer* destBuffer, const unabto_abuffer *srcBuffer)
{
    if (NULL == destBuffer || NULL == srcBuffer || UNABTO_ABUFFER_GET_SIZE(destBuffer) < UNABTO_ABUFFER_GET_USED(srcBuffer))
    {
        return false;
    }
    memcpy(UNABTO_ABUFFER_GET_HEAD(destBuffer), (const void*) UNABTO_ABUFFER_GET_HEAD(srcBuffer), UNABTO_ABUFFER_GET_UNUSED(srcBuffer));
    UNABTO_ABUFFER_SET_USED(destBuffer, 0);
    return true;
}

bool unabto_abuffer_add_data(unabto_abuffer* aBuffer, const uint8_t *data, uint16_t dataLength)
{
    if (NULL == aBuffer || UNABTO_ABUFFER_GET_UNUSED(aBuffer) < dataLength)
    {
        return false;
    } 
    memcpy(UNABTO_ABUFFER_GET_HEAD(aBuffer), data, dataLength);
    UNABTO_ABUFFER_ADVANCE(aBuffer, dataLength);
    return true;
}


bool unabto_abuffer_add_str(unabto_abuffer* aBuffer, const char *str)
{
    return unabto_abuffer_add_data(aBuffer, (uint8_t*)str, (uint16_t)strlen(str));
}


bool buffer_abuffer_add_str_terminator(unabto_abuffer* aBuffer)
{
    uint8_t temp=0;
    return unabto_abuffer_add_data(aBuffer, &temp, 1); /* not the fastes way to do it */
}


bool unabto_abuffer_add_buffer(unabto_abuffer* aBuffer, const unabto_buffer* buffer)
{
    return unabto_abuffer_add_data(aBuffer, unabto_buffer_get_data(buffer), unabto_buffer_get_size(buffer)); 
}
