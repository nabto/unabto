#include <unabto/unabto_external_environment.h>

nabto_stamp_t nabtoGetStamp() {
    struct timespec res;
    clock_gettime(CLOCK_MONOTONIC, &res);
    return res.tv_sec+(res.tv_nsec/1000000);
}

bool nabtoIsStampPassed(nabto_stamp_t* stamp) {
    nabto_stamp_t now = nabtoGetStamp();
    return *stamp <= now;
}

nabto_stamp_diff_t nabtoStampDiff(nabto_stamp_t * newest, nabto_stamp_t * oldest) {
    return newest - oldest;
}

int nabtoStampDiff2ms(nabto_stamp_diff_t diff) {
    return (int) diff;
}

