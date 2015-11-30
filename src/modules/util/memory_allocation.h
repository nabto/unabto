/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _MEMORY_ALLOCATION_H_
#define _MEMORY_ALLOCATION_H_

#include <unabto/unabto_env_base.h>

#ifdef __cplusplus
extern "C" {
#endif

void* checked_malloc(size_t size);
void checked_free(void* pointer);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
