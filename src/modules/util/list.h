/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _LIST_H_
#define _LIST_H_

#include "unabto/unabto_env_base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct list_element
{
    struct list_element* next;
} list_element;

typedef struct list
{
    list_element* head;
    list_element* tail;
    size_t count;
} list;

void list_clear(list* list);
void list_convert_array_to_list(list* list, void* store, size_t storeSize, size_t elementSize);

void list_append(list* list, void* element);
void list_remove(list* list, void* element);
void* list_remove_first(list* list);

void* list_first(list* list);
bool list_is_empty(list* list);
size_t list_count(list* list);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
