/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
// This is a loop back link layer for the NSLP protocol.
// It emulates a frame-based link between two threads running in the same process.

//#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_NSLP

#include "nslp_binary_link.h"
#include "nslp_binary_transport.h"

#define QUEUE_LENGTH 16

typedef struct
{
  bool read;
  DWORD source;
  uint8_t data[NSLP_BINARY_LINK_MAXIMUM_FRAME_SIZE];
  uint16_t length;
} queued_frame;

static queued_frame receivedFrames[QUEUE_LENGTH];
static uint8_t receivedHead = 0;
static uint8_t receivedTail = 0;
static uint8_t receivedCount = 0;
static CRITICAL_SECTION criticalSection;
static bool initialized = false;

//#define NOISE_FACTOR 6

#if NOISE_FACTOR
NABTO_THREAD_LOCAL_STORAGE static uint8_t noise;
#endif

void nslp_binary_link_initialize(void)
{
  if(initialized == false)
  {
    initialized = true;
    InitializeCriticalSectionAndSpinCount(&criticalSection, 10000);
  }

#if NOISE_FACTOR
  noise = GetCurrentThreadId() % NOISE_FACTOR;
#endif
}

void nslp_binary_link_tick(void)
{ }

void nslp_binary_link_send(uint8_t* frame, uint16_t length)
{
  DWORD threadId = GetCurrentThreadId();
  
  EnterCriticalSection(&criticalSection);
  
  if(receivedCount < QUEUE_LENGTH && length <= NSLP_BINARY_LINK_MAXIMUM_FRAME_SIZE)
  {
    queued_frame* f = &receivedFrames[receivedHead];

    f->read = false;
    f->source = threadId;
    memcpy(f->data, frame, length);
    f->length = length;
    
#if NOISE_FACTOR
  if(++noise == NOISE_FACTOR)
  {
    noise = 0;
    f->data[0] += 1;
    NABTO_LOG_TRACE(("[%lu] Added noise to frame.", GetCurrentThreadId()));
  }
#endif

    receivedHead = (receivedHead + 1) % QUEUE_LENGTH;
    receivedCount++;
    
    NABTO_LOG_TRACE(("[%lu] Sent frame.", GetCurrentThreadId()));
  }
  
  LeaveCriticalSection(&criticalSection);
}

uint16_t nslp_binary_link_receive(uint8_t** frame)
{
  uint16_t length = 0;
  DWORD threadId = GetCurrentThreadId();
  
  EnterCriticalSection(&criticalSection);

  // remove frames that have been read
  while(receivedCount > 0 && receivedFrames[receivedTail].read && receivedFrames[receivedTail].source != threadId)
  {
    receivedTail = (receivedTail + 1) % QUEUE_LENGTH;
    receivedCount--;
    NABTO_LOG_TRACE(("[%lu] Removed frame.", GetCurrentThreadId()));
  }
  
  if(receivedCount > 0)
  {
    queued_frame* f = &receivedFrames[receivedTail];
    if(f->source != threadId)
    {
      *frame = f->data;
      length = f->length;
      f->read = true;
      Sleep(2); // simulate delay - also avoids log lines being mixed up
      NABTO_LOG_TRACE(("[%lu] Received frame.", GetCurrentThreadId()));
    }
  }

  LeaveCriticalSection(&criticalSection);
  
  return length;
}
