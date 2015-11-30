/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#include <modules/log/unabto_log_header.h>
#include <modules/log/unabto_basename.h>


#include <windows.h>
#include <string.h>
#include <stdio.h>

/* Too many projects need to include modules/log/unabto_basename.c, so we
 * simply implement a windows version of unabto_basename here */
static const char* win_unabto_basename(const char* path)
{
    const char *p;
    char ch;

    p = path + strlen(path);
    while (p > path) {
        ch = *(p - 1);
        if (ch == '/' || ch == '\\' || ch == ':')
            break;
        --p;
    }
    return p;
}


int unabto_log_header(const char * file, unsigned int line)
{
    SYSTEMTIME st;
    GetSystemTime(&st);
    return printf("%02u:%02u:%02u:%03u %s(%u) ",
                  st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
                  win_unabto_basename(file), line);
}
