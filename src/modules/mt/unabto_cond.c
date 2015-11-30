/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include "unabto_cond.h"

int unabto_cond_init(unabto_cond_t * cond) {
#ifdef WIN32
    cond->signal = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (cond->signal == NULL) return -1;
    cond->broadcast = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (cond->broadcast == NULL) {
        CloseHandle(cond->signal);
        cond->signal = NULL;
        return -1;
    }
    return 0;
#else
    return pthread_cond_init(cond, NULL);
#endif
}

int unabto_cond_destroy(unabto_cond_t * cond) {
#ifdef WIN32
    return CloseHandle(cond->signal) && CloseHandle(cond->broadcast) ? 0 : -1;
#else
    return pthread_cond_destroy(cond);
#endif
}

int unabto_cond_wait(unabto_cond_t * cond, unabto_mutex_t * mutex) {
#ifdef WIN32
    HANDLE handles[] = {cond->signal, cond->broadcast};
    ReleaseMutex(*mutex);
    WaitForMultipleObjects(2, handles, FALSE, INFINITE);
    return WaitForSingleObject(*mutex, INFINITE) == WAIT_OBJECT_0? 0 : -1;
#else
    return pthread_cond_wait(cond, mutex);
#endif
}

int unabto_cond_signal(unabto_cond_t * cond) {
#ifdef WIN32
    return (SetEvent(cond->signal) == 0) ? -1 : 0;
#else
    return pthread_cond_signal(cond);
#endif
}

int unabto_cond_broadcast(unabto_cond_t * cond) {
#ifdef WIN32
    // Implementation with PulseEvent() has race condition, see
    // http://www.cs.wustl.edu/~schmidt/win32-cv-1.html
    return (PulseEvent(cond->broadcast) == 0) ? -1 : 0;
#else
    return pthread_cond_broadcast(cond);
#endif
}

