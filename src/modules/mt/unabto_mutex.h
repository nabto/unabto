/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_MT_MUTEX_H_
#define _UNABTO_MT_MUTEX_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
typedef HANDLE unabto_mutex_t;
#define DECLARE_UNABTO_MUTEX(m) unabto_mutex_t m = NULL
#else
typedef pthread_mutex_t unabto_mutex_t;
#define DECLARE_UNABTO_MUTEX(m) unabto_mutex_t m = PTHREAD_MUTEX_INITIALIZER
#endif

extern int unabto_mutex_init(unabto_mutex_t * mutex);
extern int unabto_mutex_destroy(unabto_mutex_t * mutex);
extern int unabto_mutex_lock(unabto_mutex_t * mutex);
extern int unabto_mutex_trylock(unabto_mutex_t * mutex);
extern int unabto_mutex_unlock(unabto_mutex_t * mutex);


/*********************************************************************/
/*********************************************************************/

#ifdef WIN32
typedef CRITICAL_SECTION unabto_lmutex_t;
#else
typedef unabto_mutex_t unabto_lmutex_t;
#endif

#ifdef WIN32
#define unabto_lmutex_init(m)     InitializeCriticalSection(m)
#define unabto_lmutex_destroy(m)  DeleteCriticalSection(m)
#define unabto_lmutex_lock(m)     EnterCriticalSection(m)
#define unabto_lmutex_trylock(m)  TryEnterCriticalSection(m)
#define unabto_lmutex_unlock(m)   LeaveCriticalSection(m)
#else
#define unabto_lmutex_init(m)     ((void)unabto_mutex_init(m))
#define unabto_lmutex_destroy(m)  ((void)unabto_mutex_destroy(m))
#define unabto_lmutex_lock(m)     ((void)unabto_mutex_lock(m))
#define unabto_lmutex_trylock(m)  (unabto_mutex_trylock(m) == 0)
#define unabto_lmutex_unlock(m)   ((void)unabto_mutex_unlock(m))
#endif


#ifdef __cplusplus
} //extern "C"
#endif

#endif
