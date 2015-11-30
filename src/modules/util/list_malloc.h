/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _LIST_MALLOC_H_
#define _LIST_MALLOC_H_

#include <unabto/unabto_env_base.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct list_element
{
    struct list_element* next;
    void* data;
} list_element;

typedef struct list
{
    list_element* head;
    list_element* tail;
    size_t count;
} list;

void list_initialize(list* list);

bool list_append(list* list, void* data);
bool list_remove(list* list, void* data);
bool list_remove_first(list* list, void** data);
bool list_remove_all(list* list);
bool list_peek_first(list* list, void** data);
bool list_is_empty(list* list);
size_t list_count(list* list);
bool list_contains(list* list, void* data);

#define list_foreach(_list, _element) for((_element) = (_list)->head; (_element) != NULL; (_element) = (_element)->next)


#ifdef __cplusplus
} //extern "C"
#endif

#endif
