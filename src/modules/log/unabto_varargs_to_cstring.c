/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#include "unabto_varargs_to_cstring.h"

#include <stdio.h>

static char print_buffer[1024];

char* cppstyle(const char *fmt, ...) {
    va_list arg_ptr;
    va_start(arg_ptr, fmt);
#if defined(_MSC_VER)
    vsnprintf_s(print_buffer, sizeof(print_buffer), sizeof(print_buffer)-1, fmt, arg_ptr);
#else
    vsnprintf(print_buffer, sizeof(print_buffer), fmt, arg_ptr);
#endif
    va_end(arg_ptr);
    return print_buffer;
}
