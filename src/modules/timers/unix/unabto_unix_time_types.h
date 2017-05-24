#ifndef _UNABTO_UNIX_TIME_TYPES_H_
#define _UNABTO_UNIX_TIME_TYPES_H_

#include <time.h>
#include <stdint.h>


struct unabto_unix_time {
    uint64_t milliseconds;
    uint64_t count; // The count is used to distinguish two timestamps with equal milliseconds but with different place in time of calling nabtoGetStamp()
};

typedef struct unabto_unix_time nabto_stamp_t;
// milliseconds
typedef int64_t nabto_stamp_diff_t;

#endif
