/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#include <unabto/unabto_external_environment.h>

/* Because #warning is not a known pragma in MSC */
#define STRINGISE_IMPL(x) #x
#define STRINGISE(x) STRINGISE_IMPL(x)
#if _MSC_VER
#   define FILE_LINE_LINK __FILE__ "(" STRINGISE(__LINE__) ") : "
#   define WARN(exp) (FILE_LINE_LINK "WARNING: " exp)
#else
#   define WARN(exp) ("WARNING: " exp)
#endif

#pragma message WARN("Using an incomplete random implementation. Please use a real random implementation.")

/**
 * This is not ramdom, but a dummy implementation of the unabto_random function
 */

void nabto_random(uint8_t* buf, size_t len) {
    size_t ix;
    for (ix = 0; ix < len; ++ix) {
        *buf++ = (uint8_t)ix;
    }
}
