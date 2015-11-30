#ifndef _UNABTO_NEXT_EVENT_H_
#define _UNABTO_NEXT_EVENT_H_

#include <unabto_platform_types.h>

#ifdef __cplusplus
extern "C" {
#endif

void nabto_update_min_stamp(nabto_stamp_t* current_minimum_stamp, nabto_stamp_t* stamp);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
