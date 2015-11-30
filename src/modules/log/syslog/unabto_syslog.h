/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _SYSLOG_H_
#define _SYSLOG_H_

#include <unabto/unabto_env_base.h>

struct nabto_main_context;

#include "stdarg.h"

#ifdef __cplusplus
extern "C" {
#endif

bool unabto_syslog_init();
void unabto_syslog(uint32_t module, uint32_t severity, const char* file, unsigned int line, uint32_t syslogServer, uint16_t syslogPort, const char* format, va_list ap);

void unabto_syslog_buffer(uint32_t module, uint32_t severity, const char* file, unsigned int line, uint32_t syslogServer, uint16_t syslogPort, const uint8_t* buffer, size_t buflen, const char* format, va_list ap);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
