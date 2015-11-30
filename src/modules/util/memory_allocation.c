/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_MEMORY

#include <stdlib.h>
#include <unabto/unabto_env_base.h>

static size_t currentAllocations = 0;
static size_t peakAllocations = 0;
static size_t goodAllocations = 0;
static size_t badAllocations = 0;

void* checked_malloc(size_t size)
{
    void* pointer = malloc(size);

    if(pointer != NULL)
    {
        currentAllocations++;
        if(currentAllocations > peakAllocations)
        {
            peakAllocations = currentAllocations;
        }
        goodAllocations++;
        NABTO_LOG_TRACE(("Memory allocations (+): current=%" PRIsize ", peak=%" PRIsize ", good=%" PRIsize ", bad=%" PRIsize, currentAllocations, peakAllocations, goodAllocations, badAllocations));
    }
    else
    {
        badAllocations++;
        //NABTO_LOG_FATAL(("malloc failed!"));
        NABTO_LOG_TRACE(("Memory allocations (!): current=%" PRIsize ", peak=%" PRIsize ", good=%" PRIsize ", bad=%" PRIsize "", currentAllocations, peakAllocations, goodAllocations, badAllocations));
    }

    return pointer;
}

void checked_free(void* pointer)
{
    free(pointer);

    currentAllocations--;
    
    NABTO_LOG_TRACE(("Memory allocations (-): current=%" PRIsize ", peak=%" PRIsize ", good=%" PRIsize ", bad=%" PRIsize "", currentAllocations, peakAllocations, goodAllocations, badAllocations));
}
