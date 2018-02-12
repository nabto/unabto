/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#include <unabto/util/unabto_queue.h>
#include <unabto/unabto_util.h>

void queue_init(queue_t* q, uint8_t *buffer, uint16_t buffer_length) {
    q->buffer = buffer;
    q->capacity = buffer_length;
    queue_reset(q);
}

void queue_reset(queue_t* q) {
    q->head = 0;
    q->tail = 0;
    q->count = 0;
}

bool queue_enqueue(queue_t* q, uint8_t elm) {
    if (q->count >= q->capacity) {
        return false;
    }

    q->buffer[q->head] = elm;
    q->head = (q->head + 1) % q->capacity;

    q->count++;

    return true;
}

bool queue_enqueue_array(queue_t* q, const void* data, uint16_t length)
{
    uint8_t* p = (uint8_t*)data;

    if ((q->capacity - q->count) < length)
    {
        return false;
    }

    q->count += length;

    while (length--)
    {
        q->buffer[q->head] = *p++;
        q->head = (q->head + 1) % q->capacity;
    }

    return true;
}

bool queue_dequeue(queue_t* q, uint8_t* elm) {
    if (q->count == 0) {
        return false;
    }

    *elm = q->buffer[q->tail];
    q->tail = (q->tail + 1) % q->capacity;

    q->count--;

    return true;
}

bool queue_dequeue_array(queue_t* q, void* data, uint16_t length)
{
    uint8_t* p = (uint8_t*)data;

    if (q->count < length)
    {
        return false;
    }

    q->count -= length;

    while (length--)
    {
        *p++ = q->buffer[q->tail];
        q->tail = (q->tail + 1) % q->capacity;
    }

    return true;
}

bool queue_is_empty(queue_t* q) {
    return q->count == 0;
}

uint16_t queue_count(queue_t* q) {
    return q->count;
}

uint16_t queue_free(queue_t* q) {
    return q->capacity - q->count;
}
