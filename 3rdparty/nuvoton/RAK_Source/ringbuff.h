/**************************************************************************//**
 * @file        ringbuff.h
 * @version     V1.00
 * $Revision:   1$
 * $Date:       14/07/10 5:00p$
 * @brief       Ring buffer header file
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#ifndef __RINGBUFF_H__
#define __RINGBUFF_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct RingBuff {
    void *buff;
    unsigned size;
    unsigned max_size;
    unsigned wrt_ind;
    unsigned rd_ind;
};

void rb_init(struct RingBuff *rb, void *buff, unsigned max_size);
int rb_empty(struct RingBuff *rb);
int rb_full(struct RingBuff *rb);
void rb_next_write(struct RingBuff *rb, void **next_wrt_p, unsigned *next_wrt_cap);
void rb_write_done(struct RingBuff *rb, unsigned n);
void rb_next_read(struct RingBuff *rb, void **next_rd_p, unsigned *next_rd_cap);
void rb_read_done(struct RingBuff *rb, unsigned n);

#ifdef __cplusplus
}
#endif

#endif //__RINGBUFF_H__

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/
