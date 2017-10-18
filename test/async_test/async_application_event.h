#ifndef _ASYNC_APPLICATION_EVENT_H_
#define _ASYNC_APPLICATION_EVENT_H_

#include <unabto_platform_types.h>

void update_next_async_event_timeout(nabto_stamp_t* ne);
void init_request_queue();

#endif
