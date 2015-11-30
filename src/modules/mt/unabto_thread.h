/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_MT_THREAD_H_
#define _UNABTO_MT_THREAD_H_

#ifndef WIN32
#include <pthread.h>
#endif
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
typedef HANDLE unabto_thread_t;
#else
typedef pthread_t unabto_thread_t;
#endif

typedef intptr_t (* unabto_thread_func)(void * data);

extern int  unabto_thread_create(unabto_thread_t * thread, unabto_thread_func start, void * data);
extern int  unabto_thread_detach(unabto_thread_t thread);
extern int  unabto_thread_join(unabto_thread_t thread, intptr_t * retval, int * canceled);
extern void unabto_thread_exit(intptr_t retval);
extern int  unabto_thread_cancel(unabto_thread_t thread);

extern unabto_thread_t unabto_thread_self();
extern int unabto_thread_is_self(unabto_thread_t thread);
extern int unabto_thread_equal(unabto_thread_t thread1, unabto_thread_t thread2);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
