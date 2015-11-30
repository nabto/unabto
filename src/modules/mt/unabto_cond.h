/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_MT_COND_H_
#define _UNABTO_MT_COND_H_

#include "unabto_mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
typedef struct {
    HANDLE signal;
    HANDLE broadcast;
} unabto_cond_t;
#define DECLARE_UNABTO_COND(c) unabto_cond_t c = {NULL, NULL}
#else
typedef pthread_cond_t unabto_cond_t;
#define DECLARE_UNABTO_COND(c) unabto_cond_t c = PTHREAD_COND_INITIALIZER
#endif

extern int unabto_cond_init(unabto_cond_t * cond);
extern int unabto_cond_destroy(unabto_cond_t * cond);
extern int unabto_cond_wait(unabto_cond_t * cond, unabto_mutex_t * mutex);
extern int unabto_cond_signal(unabto_cond_t * cond);
extern int unabto_cond_broadcast(unabto_cond_t * cond);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
