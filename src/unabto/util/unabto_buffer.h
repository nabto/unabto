/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
/**
 * @file
 * Deprecated buffer definitions
 * 
 * For uNabto query read and writing use the unabto_query files
 * For the buffer_t type, use the unabto_buffers files
 * 
 */
 
#ifndef _UNABTO_BUFFER_H_
#define _UNABTO_BUFFER_H_

#include <unabto/unabto_env_base.h>
#include <unabto/unabto_buffers.h>
#include <unabto/unabto_query_rw.h>
#include <unabto/util/unabto_queue.h>
#include <unabto/unabto_buffers.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unabto_buffer buffer_t; 


/**********************************/
/* Prototypes for the buffer type */
/**********************************/

/** Deprecated - use unabto_buffer_init
 * Initialize the buffer. Must be called before any other calls to the buffer.
 * @param buffer  the target buffer
 * @param data    the new content or just space for usage
 * @param size    the size of the new content or available space
 */
#define buffer_init(buffer, data, size) unabto_buffer_init(buffer, data, size) 


/** Deprecated - use unabto_buffer_get_data
 * Get the raw buffere.
 * @param buffer     the target buffer
 * @return a pointer to the raw data buffer
 */
#define buffer_get_data(buffer) unabto_buffer_get_data(buffer)


/** Deprecated - use unabto_buffer_get_size
 * Get the size of buffer.
 * @param buffer     the target buffer
 * @return size
 */
#define buffer_get_size(buffer) unabto_buffer_get_size(buffer) 


/** Deprecated - use unabto_buffer_copy
 * copy a buffer to another buffer
 * @param dest  destination buffer
 * @param src   source buffer
 * @return true if room
 */
#define buffer_copy(dest, src) unabto_buffer_copy(dest, src)

/** Deprecated - use unabto_buffer_cmp
 * string compares a buffer with another buffer
 * @param b1  buffer
 * @param b2  buffer
 * @return 0 - equal, -1 - b1 < b2, 1 b1 > b2,     
 */
#define buffer_cmp(b1, b2) unabto_buffer_cmp(b1, b2)



typedef unabto_query_request buffer_read_t;    
typedef unabto_query_response buffer_write_t;
//#define buffer_write_t unabto_query_response     

/***************************************************/
/* Prototypes for application query buffer reading */
/***************************************************/

/** Deprecated - use unabto_query_read_uint32
 * read an uint32_t from a readable buffer
 * @param read_buf  the readable buffer to read from
 * @param res       the resulting uint32_t
 * @return true on success
 */
#define buffer_read_uint32(read_buf, res) unabto_query_read_uint32(read_buf, res)


/** Deprecated - use unabto_query_read_uint16
 * read an uint16_t from a readable buffer
 * @param read_buf  the readable buffer to read from
 * @param res       the resulting uint8_t
 * @return true on success
 */
#define buffer_read_uint16(read_buf, res) unabto_query_read_uint16(read_buf, res)


/** Deprecated - use unabto_query_read_uint8
 * read an uint16_t from a readable buffer
 * @param read_buf  the readable buffer to read from
 * @param res       the resulting uint8_t
 * @return true on success
 */
#define buffer_read_uint8(read_buf, res) unabto_query_read_uint8(read_buf, res)


/** Deprecated - use unabto_query_read_uint8_list
 * read raw data from a buffer
 * @param read_buf  the reaable buffer
 * @param buf       the buffer to write to
 * @return true if success
 */
bool buffer_read_raw(buffer_read_t* read_buf, buffer_t* buf);


/** Deprecated - use unabto_query_read_uint8_list
 * read raw data from a buffer without copying data
 * @param read_buf  the reaable buffer
 * @param buf       the buffer to write to
 * @return true if success
 */
bool buffer_read_raw_nc(buffer_read_t* read_buf, buffer_t* buf);

/** Deprecated - use nabto_query_read_reset
 * Reset a a readable buffer to read from start
 * @param read_buf  the buffer to reset.
 */

/** Deprecated
 * initialize read buffer
 */
void buffer_read_init(buffer_read_t* read_buf, buffer_t* buf);

#define buffer_read_reset(buffer) unabto_abuffer_reset(buffer)


/*******************************************************************************/
/* Prototypes for unabto and optionally application query read buffer handling */
/*******************************************************************************/

/** Deprecated - use unabto_abuffer_get_size ...
 * Get the number of available bytes in the read buffer.
 * @param read_buf the readable buffer.
 * @return the number of bytes which the read buffers holds.
 */
#define buffer_read_available(read_buf) unabto_abuffer_get_size(read_buf)


/** Deprecated - use unabto_abuffer_get_unused ...
 * Get the number of bytes that can be read from the buffer.
 * @param read_buf the readable buffer.
 * @return the number of bytes which have not yet be read from the buffer.
 */
#define buffer_read_remaining(read_buf) unabto_abuffer_get_unused(read_buf)


/** Deprecated  - use unabto_abuffer_get_head 
 * return the head of the buffer we read from 
 * @param  read_buf the buffer to find the head on.
 * @return the pointer to the head.
 */
#define buffer_read_head(read_buf) unabto_query_get_read_head(read_buf)


/** Deprecated - use unabto_abuffer_get_size
 * Is the buffer empty.
 * @param buffer     the target buffer
 * @return true if buffer is empty
 */
#define buffer_read_is_empty(buffer) (0 == unabto_abuffer_get_size(buffer))


/***************************************************/
/* Prototypes for application query buffer writing */
/***************************************************/

/** Deprecated - use unabto_query_write_uint8_list
 * write a buffer to a writeable buffer encoded as 16bit length and data.
 * @param buffer     the target buffer
 * @param buffer_in  the buffer to write
 * @return true on success
 */
#define buffer_write_raw(writeBuffer, buffer_in)   unabto_query_write_uint8_list(writeBuffer, unabto_buffer_get_data(buffer_in), unabto_buffer_get_size(buffer_in))


/** Deprecated - use unabto_query_write_uint8_list
 * write an array to a writeable buffer encoded as 16bit length and data.
 * @param w_buf      the target buffer
 * @param data       the source array
 * @param length     the amount of data to write
 * @return true on successDeprecated - use unabto_buffer_
 */
#define buffer_write_raw_from_array(w_buf, data, length) unabto_query_write_uint8_list(w_buf, data, length)


/** Deprecated - use unabto_query_write_uint8
 * Append an uint8_t to the buffer
 * @param buffer     the target buffer
 * @param elm        the uint8_t
 * @return false if the operation would have resulted in a buffer overflow.
 */
#define buffer_write_uint8(buffer, elm) unabto_query_write_uint8(buffer, elm)


/** Deprecated - use unabto_query_write_uint16
 * Append an uint16_t to the buffer
 * @param buffer     the target buffer
 * @param elm        the uint16_t
 * @return false if the operation would have resulted in a buffer overflow.
 */
#define buffer_write_uint16(buffer, elm) unabto_query_write_uint16(buffer, elm)


/** Deprecated - use unabto_query_write_uint32
 * Append an uint32_t to the buffer
 * @param buffer     the target buffer
 * @param elm        the uint32_t
 * @return false if the operation would have resulted in a buffer overflow.
 */
#define buffer_write_uint32(buffer, elm) unabto_query_write_uint32(buffer, elm)


/*************************************************************************************/
/* Prototypes for unabto core and optionally application query write buffer handling */
/*************************************************************************************/

/** Deprecated - use internal function unabto_abuffer_reset
 * Resets the buffer to the empty state.
 * @param buffer     the target buffer
 */
#define buffer_write_reset(buffer) unabto_abuffer_reset(buffer)


/**
 * Initialise a writeable buffer.
 */
/*
#define buffer_write_init(b_w, buf) unabto_query_write_init(b_w, unabto_buffer_get_data(buf), unabto_buffer_get_size(buf))
*/
#define buffer_write_init(b_w, buf) unabto_query_response_init(b_w, buf)


/**
 * Return the head of the buffer.
 * @param buffer     the target buffer
 * @return a pointer to the head of the buffer.
 */
#define buffer_write_head(buffer)  unabto_query_get_write_head(buffer)


#define buffer_write_used(buffer) unabto_query_get_write_used(buffer)


#define buffer_write_data(buffer) unabto_query_get_write_initial_head(buffer)

/***************************************************************/
/* Prototypes for application non query related buffer writing */
/***************************************************************/

/** Deprecated - use buffer_abuffer_add_str_terminator
 * Null terminates a buffer if there's room for it.
 * @param buffer     the target buffer
 * @return true if room
 */
#define buffer_write_null_terminate(buffer) unabto_abuffer_add_str_terminator(buffer)


/** Deprecated - use unabto_abuffer_add_str
 * Append a null terminated string to a buffer
 * @param buffer     the target buffer
 * @param str        the string
 * @return true if room
 */
#define buffer_write_str(buffer, str) unabto_abuffer_add_str(buffer, str)


/** Deprecated - use unabto_abuffer_add_buffer
 * Append buffer_in to the buffer. 
 * @param buffer     the target buffer
 * @param buffer_in  the buffer to be appended
 * @return false if the operation would have resulted in a buffer overflow
 */
#define buffer_write_buffer(writeBuffer, buffer_in) unabto_abuffer_add_buffer(writeBuffer, buffer_in);


/** Deprecated - use buffer_abuffer_add_str_terminator
 * Null terminates a buffer if there's room for it.
 * @param buffer     the target buffer
 * @return true if room
 */
#define buffer_write_null_terminate(buffer) unabto_abuffer_add_str_terminator(buffer)


/***********************************************************************************************/
/* Prototypes for application and platform specific query and non query related buffer writing */
/***********************************************************************************************/

#if UNABTO_PLATFORM_PIC18
/** Deprecated - no substitute here. check in the PIC18 platform
 * Append a null terminated string to a buffer
 * @param buffer     the target buffer
 * @param str        the string
 * @return true if room
 */
bool buffer_write_str_pgm(buffer_write_t *buffer, text str);


/** Deprecated - no substitute here check in PIC18 platform
 * Append a buffer from program space to the buffer,
 * The length is prefixed as a uint16
 */
bool buffer_write_raw_pgm(buffer_write_t* w_buf, const __ROM uint8_t* src, uint16_t len);

#define buffer_write_text(buffer, string) buffer_write_str_pgm(buffer, string)

#else

#define buffer_write_text(buffer, string) buffer_write_str(buffer, string)

#endif


/** Deprecated - no substitute check in modules
 * Append all contents of a queue into the buffer.
 * @param w_buf  the target buffer
 * @param q      the queue
 * @return       true iff succeessfully written
 * The length is prefixed as a uint16
 */
bool buffer_write_raw_from_queue(buffer_write_t* w_buf, queue_t* q);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
