/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_MEMORY

#include <modules/util/list_malloc.h>
#include <modules/util/memory_allocation.h>

void list_initialize(list* list)
{
    UNABTO_ASSERT(list != NULL);

    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}

bool list_append(list* list, void* data)
{
    list_element* e;
    
    UNABTO_ASSERT(list != NULL);

    e = (list_element*)checked_malloc(sizeof(list_element));

    if(e == NULL)
    {
        NABTO_LOG_ERROR(("malloc failed!"));
        return false;
    }

    e->data = data;
    e->next = NULL;

    if(list->head == NULL)
    {
        list->head = e;
        list->tail = e;
    }
    else
    {
        list->tail->next = e;
        list->tail = e;
    }

    list->count++;

    return true;
}

bool list_remove(list* list, void* data)
{
    list_element* current;
    list_element* previous;
    
    UNABTO_ASSERT(list != NULL);

    if(list->head != NULL) // list is not empty
    {
        current = list->head;
        previous = NULL;

        do
        {
            if(current->data == data) // is this the element to remove
            {
                if(current == list->head) // first element in list
                {
                    if(current == list->tail) // also the last element in the list
                    {
                        list->head = NULL;
                        list->tail = NULL;
                    }
                    else
                    {
                        list->head = current->next; // advance head pointer
                    }
                }
                else if(current == list->tail) // last element in list
                {
                    previous->next = NULL;
                    list->tail = previous;
                }
                else
                {
                    previous->next = current->next;
                }
            
                checked_free(current);

                list->count--;

                return true;
            }

            previous = current;
            current = current->next;
        } while(current != NULL);
    }

    NABTO_LOG_ERROR(("Tried to remove element not in list!"));

    return false;
}

bool list_remove_first(list* list, void** data)
{
    list_element* e;
    
    UNABTO_ASSERT(list != NULL);

    if(list->head == NULL)
    {
        return false;
    }

    e = list->head;

    if(data != NULL)
    {
        *data = e->data;
    }

    if(list->head == list->tail) // is it the only element in the list
    {
        list->head = NULL;
        list->tail = NULL;
    }
    else
    {
        list->head = list->head->next;
    }

    checked_free(e);
        
    list->count--;

    return true;
}

bool list_remove_all(list* list)
{
    list_element* current;
    list_element* next;
    
    UNABTO_ASSERT(list != NULL);

    if(list->head == NULL)
    {
        return true;
    }

    current = list->head;

    while(current != NULL)
    {
        next = current->next;
        checked_free(current);
        current = next;
    }

    list->head = NULL;
    list->tail = NULL;
    list->count = 0;

    return true;
}

bool list_peek_first(list* list, void** data)
{
    UNABTO_ASSERT(list != NULL);

    if(list->head == NULL)
    {
        return false;
    }

    *data = list->head->data;

    return true;
}

bool list_is_empty(list* list)
{
    UNABTO_ASSERT(list != NULL);

    return list->count == 0;
}

size_t list_count(list* list)
{
    UNABTO_ASSERT(list != NULL);

    return list->count;
}

bool list_contains(list* list, void* data)
{
    list_element* e;
    
    UNABTO_ASSERT(list != NULL);

    //list_foreach(list, e)
    //{
    //    if(e->data == data)
    //    {
    //        return true;
    //    }
    //}

    e = list->head;

    while(e != NULL)
    {
        if(e->data == data)
        {
            return true;
        }

        e = e->next;
    }

    return false;
}
