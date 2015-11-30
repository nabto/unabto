/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include "unabto_thread.h"

#include <malloc.h>

struct unabto_thread_data {
    unabto_thread_func start;
    void * data;
};

#ifdef WIN32
#define WIN32_THREAD_CANCEL ((DWORD)(-1))
#endif

#ifdef WIN32
static DWORD WINAPI thread_func(LPVOID data)
{
    unabto_thread_func func_addr = (unabto_thread_func) data;
    return (DWORD)(*func_addr)(NULL);
}
#else
static void * thread_func(void * data)
{
    unabto_thread_func func_addr = (unabto_thread_func) data;
    return (void *)(*func_addr)(NULL);
}
#endif

#ifdef WIN32
static DWORD WINAPI thread_body(LPVOID data)
#else
static void * thread_body(void * data)
#endif
{
    struct unabto_thread_data * thread_data = (struct unabto_thread_data *) data;
    unabto_thread_func func_addr = thread_data->start;
    void * func_data = thread_data->data;
    free(thread_data);
#ifdef WIN32
    return (DWORD)(*func_addr)(func_data);
#else
    return (void *)(*func_addr)(func_data);
#endif
}

int unabto_thread_create(unabto_thread_t * thread, unabto_thread_func start, void * data)
{
#ifdef WIN32
    HANDLE h;
#else
    int res;
#endif

    if (data) {
        struct unabto_thread_data * thread_data;

        thread_data = (struct unabto_thread_data *) malloc(sizeof(struct unabto_thread_data));
        if (!thread_data)
            return -1;
        thread_data->start = start;
        thread_data->data = data;

#ifdef WIN32
        h = CreateThread(NULL, 0, thread_body, thread_data, 0, NULL);
        if (h == NULL) {
            free(thread_data);
            return -1;
        }
        *thread = h;
#else
        res = pthread_create(thread, NULL, thread_body, thread_data);
        if (res != 0) {
            free(thread_data);
            return res;
        }
#endif
    } else {
#ifdef WIN32
        h = CreateThread(NULL, 0, thread_func, start, 0, NULL);
        if (h == NULL) return -1;
        *thread = h;
#else
        res = pthread_create(thread, NULL, thread_func, start);
        if (res != 0) return res;
#endif
    }

    return 0;
}

int unabto_thread_detach(unabto_thread_t thread)
{
#ifdef WIN32
    return 0;
#else
    return pthread_detach(thread);
#endif
}

int unabto_thread_join(unabto_thread_t thread, intptr_t * retval, int *canceled)
{
#ifdef WIN32
    DWORD code;
    if (WaitForSingleObject(thread, INFINITE) != WAIT_OBJECT_0)
        return -1;
    if (!GetExitCodeThread(thread, &code))
        code = 0;
    if (retval)
        *retval = (int) code;
    if (canceled)
        *canceled = (code == WIN32_THREAD_CANCEL);
    return 0;
#else
    int res;
    void *code;
    res = pthread_join(thread, &code);
    if (res != 0)
        return res;
    if (retval)
        *retval = (intptr_t) code;
    if (canceled)
        *canceled = (code == PTHREAD_CANCELED);
    return 0;
#endif
}

void unabto_thread_exit(intptr_t retval)
{
#ifdef WIN32
    ExitThread((DWORD) retval);
#else
    pthread_exit((void *) retval);
#endif
}

int unabto_thread_cancel(unabto_thread_t thread)
{
#ifdef WIN32
    return TerminateThread(thread, WIN32_THREAD_CANCEL) ? 0 : -1;
#else
    return pthread_cancel(thread);
#endif
}

unabto_thread_t unabto_thread_self()
{
#ifdef WIN32
    return GetCurrentThread();
#else
    return pthread_self();
#endif
}

int unabto_thread_is_self(unabto_thread_t thread)
{
#ifdef WIN32
    return GetCurrentThreadId() == GetThreadId(thread);
#else
    return pthread_self() == thread;
#endif
}

int unabto_thread_equal(unabto_thread_t thread1, unabto_thread_t thread2)
{
#ifdef WIN32
    return GetThreadId(thread1) == GetThreadId(thread2);
#else
    return pthread_equal(thread1, thread2);
#endif
}
