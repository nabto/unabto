/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include "unabto_mutex.h"

int unabto_mutex_init(unabto_mutex_t * mutex) {
#ifdef WIN32
    *mutex = CreateMutex(NULL, FALSE, NULL);
    return *mutex == NULL ? -1 : 0;
#else
    return pthread_mutex_init(mutex, NULL);
#endif
}

int unabto_mutex_destroy(unabto_mutex_t * mutex) {
#ifdef WIN32
    return (CloseHandle(*mutex) == 0) ? -1 : 0;
#else
    return pthread_mutex_destroy(mutex);
#endif
}

int unabto_mutex_lock(unabto_mutex_t * mutex) {
#ifdef WIN32
    return (WaitForSingleObject(*mutex, INFINITE) == WAIT_OBJECT_0) ? 0 : -1;
#else
    return pthread_mutex_lock(mutex);
#endif
}

int unabto_mutex_trylock(unabto_mutex_t * mutex) {
#ifdef WIN32
    return (WaitForSingleObject(*mutex, 0) == WAIT_OBJECT_0) ? 0 : -1;
#else
    return pthread_mutex_trylock(mutex);
#endif
}

int unabto_mutex_unlock(unabto_mutex_t * mutex) {
#ifdef WIN32
    return (ReleaseMutex(*mutex) == 0) ? -1 : 0;
#else
    return pthread_mutex_unlock(mutex);
#endif
}

