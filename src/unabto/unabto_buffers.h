/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
/**
 * @file
 * Definition of a buffer data structures
 * Usage: 
 *   Call init_buffer first to initialize the buffer data structure with a memory area.
 *   Afterward buffer_reset will reset the buffer data structure; 
 */
 
#ifndef _UNABTO_BUFFERS_H_
#define _UNABTO_BUFFERS_H_

#include "unabto/unabto_env_base.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A buffer access type.
 */
typedef struct {
    uint16_t size;   /**< the size of the buffer                    */
    uint8_t* data; /**< the content                               */
} unabto_buffer;


/**************************/
/* Prototypes for buffers */
/**************************/

/**
 * Initialize the buffer. Must be called before any other calls to the buffer.
 * @param buffer  the target buffer
 * @param data    the new content
 * @param size    the size of the new content
 */
void unabto_buffer_init(unabto_buffer* buffer, uint8_t *data, uint16_t size);


/**
 * Get the raw buffer.
 * @param buffer     the target buffer
 * @return a pointer to the raw data buffer
 */
uint8_t* unabto_buffer_get_data(const unabto_buffer* buffer);


/**
 * Get the size of buffer.
 * @param buffer     the target buffer
 * @return size
 */
int unabto_buffer_get_size(const unabto_buffer* buffer);


/**
 * copy a buffer to another buffer
 * @param dest  destination buffer
 * @param src   source buffer
 * @return true if room
 */
bool unabto_buffer_copy(unabto_buffer* dest, const unabto_buffer* src);


/**
 * string compares a buffer with another buffer
 * @param b1  buffer
 * @param b2  buffer
 * @return 0 - equal, -1 - b1 < b2, 1 b1 > b2,     
 */
int unabto_buffer_cmp(const unabto_buffer *b1, const unabto_buffer *b2);


/** move out **/


/*
    abuffer 
*/    

typedef struct {
    unabto_buffer *buffer; /**< buffer for data */
    uint16_t position; /**< pointer to current read/write location in the buffer */
} unabto_abuffer; // rename 


/** 
 * Reset the abuffer to the start
 * @param aBuffer  the abuffer to reset.
 */
bool unabto_abuffer_reset(unabto_abuffer* aBuffer);


/**
 * Initialize the abuffer. Must be called before any other calls to the buffer.
 * @param aBuffer  the target abuffer
 * @param buffer   a unabto_buffer with new content or space for adding new content
 *                 previously initialized with unabto_buffer_init
 */
bool unabto_abuffer_init(unabto_abuffer* aBuffer, unabto_buffer *buffer);


/**
 * Initialize the abuffer. Must be called before any other calls to the buffer.
 * @param aBuffer  the target abuffer
 * @param data     the new content or space for adding new content
 * @param dataSize the size of the content or the maximum space for adding content
 *
bool unabto_abuffer_init(unabto_abuffer* aBuffer, uint8_t *data, uint16_t dataSize);
*/

/** Generic application operations  */

/**
 * Get a pointer to the current read/write position of buffer
 * @param abuffer     the target abuffer
 * @return a pointer to the current buffer position
 */
uint8_t *unabto_abuffer_get_head(const unabto_abuffer* aBuffer);


/**
 * Get the total size of the abuffer.
 * @param aBuffer    the target abuffer
 * @return size
 */
uint16_t unabto_abuffer_get_size(const unabto_abuffer* aBuffer);

/**
 * Get the "used" data size of the abuffer.
 * @param aBuffer    the target abuffer
 * @return the used
 */
uint16_t unabto_abuffer_get_used(const unabto_abuffer* aBuffer);


/**
 * Get the "unused" data size of the abuffer.
 * @param aBuffer    the target abuffer
 * @return the unused
 */
uint16_t unabto_abuffer_get_unused(const unabto_abuffer* aBuffer);


/**
 * Shifts the current read/write position forward
 * @param aBuffer    the target abuffer
 * @param offset     the 
 * @return true if room
 */
bool unabto_abuffer_advance(unabto_abuffer* aBuffer, uint16_t offset);

/**
 * Copies to content and state of an abuffet to another
 * The target abuffer must have been initialized with unabto_abuffer_init prior to the function call
 * @param destBuffer    the target abuffer
 * @param srcBuffer    the source abuffer
 * @return true if room in destBuffer
 */
bool unabto_abuffer_copy(unabto_abuffer* destBuffer, const unabto_abuffer *srcBuffer);



/** Utility add data functions **/


/** 
 * Append a null terminated string to an abuffer
 * @param aBuffer     the target buffer
 * @param str        the string
 * @return true if room
 */
bool unabto_abuffer_add_str(unabto_abuffer* aBuffer, const char *str);


/** 
 * Append a C string terminator to an abuffer
 * short cut for: uint8_t t=0; unabto_abuffer_add_data(aBuffer, &t, 1); 
 * @param aBuffer     the target buffer
 * @return true if room
 */
bool buffer_abuffer_add_str_terminator(unabto_abuffer* aBuffer);


/** 
 * Append a byte array to an abuffer
 * @param aBuffer     the target buffer
 * @param data        the content to add
 * @param dataLength  the length of the content
 * @return true if room
 */
bool unabto_abuffer_add_data(unabto_abuffer* aBuffer, const uint8_t *data, uint16_t dataLength);


/** 
 * Append the contens of a buffer to an abuffer
 * @param aBuffer     the target buffer
 * @param buffer      buffer with the content to add
 * @return true if room
 */
bool unabto_abuffer_add_buffer(unabto_abuffer* aBuffer, const unabto_buffer* buffer);




/**************************************************************************
 ******** The following functions are primarily for internal usage ********
 **************************************************************************/

/* "in-liners" with no error checking */


#define UNABTO_ABUFFER_POSITION(aBuffer) (aBuffer->position)

#define UNABTO_ABUFFER_GET_USED(aBuffer) UNABTO_ABUFFER_POSITION(aBuffer)

#define UNABTO_ABUFFER_RESET(aBuffer) do {UNABTO_ABUFFER_POSITION(aBuffer) = 0; } while(0)

#define UNABTO_ABUFFER_GET_SIZE(aBuffer) (aBuffer->buffer->size)

#define UNABTO_ABUFFER_GET_UNUSED(aBuffer) ((uint16_t)(UNABTO_ABUFFER_GET_SIZE(aBuffer) - UNABTO_ABUFFER_GET_USED(aBuffer)))

#define UNABTO_ABUFFER_GET_INITIAL_HEAD(aBuffer) ((aBuffer)->buffer->data)

#define UNABTO_ABUFFER_GET_HEAD(aBuffer) (UNABTO_ABUFFER_GET_INITIAL_HEAD(aBuffer) + UNABTO_ABUFFER_POSITION(aBuffer) )

#define UNABTO_ABUFFER_ADVANCE(aBuffer, offset) do {UNABTO_ABUFFER_POSITION(aBuffer) += offset;} while(0)

#define UNABTO_ABUFFER_SET_USED(aBuffer, pos) do {UNABTO_ABUFFER_POSITION(aBuffer) = pos;} while(0)



#ifdef __cplusplus
} // extern "C"
#endif

#endif /* _UNABTO_BUFFERS_H_ */
