/**************************************************************************//**
 * @file        ringbuff.c
 * @version     V1.00
 * $Revision:   1$
 * $Date:       14/07/10 5:00p$
 * @brief       NUC505 ring buffer source file
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include "ringbuff.h"
#include <unabto/unabto.h>

void rb_init(struct RingBuff *rb, void *buff, unsigned max_size)
{
    rb->buff = buff;
    rb->size = 0;
    rb->max_size = max_size;
    rb->wrt_ind = rb->rd_ind = 0;
}

int rb_empty(struct RingBuff *rb)
{
    return (rb->size == 0) ? 1 : 0;
}

int rb_full(struct RingBuff *rb)
{
    return (rb->size == rb->max_size) ? 1 : 0;
}

void rb_next_write(struct RingBuff *rb, void **next_wrt_p, unsigned *next_wrt_cap)
{
    if (rb_full(rb)) {
        *next_wrt_p = NULL;
        *next_wrt_cap = 0;
        return;
    }

    *next_wrt_p = (void *) ((unsigned) rb->buff + rb->wrt_ind);
    if (rb->wrt_ind >= rb->rd_ind) {
        *next_wrt_cap = rb->max_size - rb->wrt_ind;
    }
    else {
        *next_wrt_cap = rb->rd_ind - rb->wrt_ind;
    }
}

void rb_write_done(struct RingBuff *rb, unsigned n)
{
    if (n > (rb->max_size - rb->size)) {
        NABTO_LOG_ERROR(("Ring buffer overflow!\n"));
    }

    rb->wrt_ind += n;
    if (rb->wrt_ind >= rb->max_size) {
        rb->wrt_ind -= rb->max_size;
    }
    rb->size += n;
}

void rb_next_read(struct RingBuff *rb, void **next_rd_p, unsigned *next_rd_cap)
{
    if (rb_empty(rb)) {
        *next_rd_p = NULL;
        *next_rd_cap = 0;
        return;
    }

    *next_rd_p = (void *) ((unsigned) rb->buff + rb->rd_ind);
    if (rb->rd_ind >= rb->wrt_ind) {
        *next_rd_cap = rb->max_size - rb->rd_ind;
    }
    else {
        *next_rd_cap = rb->wrt_ind - rb->rd_ind;
    }
}

void rb_read_done(struct RingBuff *rb, unsigned n)
{
    if (n > rb->size) {
        NABTO_LOG_ERROR(("Ring buffer underflow!\n"));
    }

    rb->rd_ind += n;
    if (rb->rd_ind >= rb->max_size) {
        rb->rd_ind -= rb->max_size;
    }
    rb->size -= n;
}

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/
