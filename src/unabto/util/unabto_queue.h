/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_QUEUE_H_
#define _UNABTO_QUEUE_H_

#include <unabto/unabto_env_base.h>

#ifdef __cplusplus
extern "C" {
#endif

/** the queue */
typedef struct queue {
    uint16_t head;     /**< head */
    uint16_t tail;     /**< tail */
    uint16_t count;    /**< number of char in queue */
    uint16_t capacity; /**< max size of queue */
    uint8_t* buffer;   /**< the buffer */
} queue_t;

/**
 * initialize the queue
 * @param q    the queue
 * @param buffer the buffer to use for internal storage
 * @param buffer_length length of the buffer
 * @return     true on success
 */
void queue_init(queue_t *q, uint8_t *buffer, uint16_t buffer_length);

/**
 * remove all data from the queue
 * @param q    the queue
 */
void queue_reset(queue_t *q);

/**
 * enqueue at the tail of the queue
 * @param q    the queue
 * @param elm  the element
 * @return     true on success
 */
bool queue_enqueue(queue_t* q, uint8_t elm);

/**
 * enqueue array at the tail of the queue
 * @param q    the queue
 * @param data  the array
 * @param length the length of the array
 * @return     true on success
 */
bool queue_enqueue_array(queue_t* q, const void* data, uint16_t length);

/**
 * dequeue from the head of the queue
 * @param q    the queue
 * @param elm  the element
 * @return     true on success
 */
bool queue_dequeue(queue_t* q, uint8_t* elm);

/**
 * dequeue array from the head of the queue
 * @param q    the queue
 * @param data  the array
 * @param length the length of the array
 * @return     true on success
 */
bool queue_dequeue_array(queue_t* q, void* data, uint16_t length);

/**
 * @return true if the queue is empty
 * @param q    the queue
 */
bool queue_is_empty(queue_t* q);

/**
 * @return the number of bytes in the queue
 * @param q    the queue
 */
uint16_t queue_count(queue_t* q);

/**
 * @return the number of bytes that can currently be enqueued into the queue
 * @param q    the queue
 */
uint16_t queue_free(queue_t* q);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
