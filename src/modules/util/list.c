/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#include "list.h"

void list_clear(list* list)
{
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}

void list_convert_array_to_list(list* list, void* store, size_t storeSize, size_t elementSize)
{
    uint16_t i;
    list_element* e;
    list_element* eNext;
    list_element* lastElement;
    uint8_t* s = (uint8_t*)store;
    uint8_t* p = s;

    memset(store, 0, storeSize);

    list->count = 0;
    
    for(i = 0; i < (storeSize / elementSize); i++)
    {
        e = (list_element*)p;
        p += elementSize;
        eNext = (list_element*)(p);

        e->next = eNext;
        list->count++;

        lastElement = e;
    }

    lastElement->next = NULL;

    list->head = (list_element*)store;
    list->tail = lastElement;
}

void list_append(list* list, void* element)
{
    list_element* e = (list_element*)element;

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

    e->next = NULL;
    
    list->count++;
}

void list_remove(list* list, void* element)
{
    list_element* e = (list_element*)element;

    if(list->head == e) // is the element the first element in the list
    {
        if(list->head == list->tail) // is it the only element in the list
        {
            list->head = NULL;
            list->tail = NULL;
        }
        else
        {
            list->head = list->head->next;
        }

        e->next = NULL;
        
        list->count--;
    }
    else if(list->head != NULL) // is the list non-empty
    {
        // move straight to element number two in the list
        list_element* previous = list->head;
        list_element* current = previous->next;
        
        while(current != NULL)
        {
            if(current == e)
            {
                previous->next = current->next; // remove element from list
                current->next = NULL;

                if(list->tail == e) // is it the last element in the list
                {
                    list->tail = previous;
                }
                
                list->count--;

                return;
            }

            previous = current;
            current = current->next;
        }

        // element was not in list
        NABTO_LOG_ERROR(("Tried to remove element not in list!"));
    }
}

void* list_remove_first(list* list)
{
    list_element* element = list->head;

    if(list->head != NULL) // list is not empty
    {
        if(list->head == list->tail) // is it the only element in the list
        {
            list->head = NULL;
            list->tail = NULL;
        }
        else
        {
            list->head = list->head->next;
        }

        element->next = NULL;
        
        list->count--;
    }

    return element;
}

void* list_first(list* list)
{
    return list->head;
}

bool list_is_empty(list* list)
{
    return list->count == 0;
}

size_t list_count(list* list)
{
    return list->count;
}
