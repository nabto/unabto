/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */

#include <unabto/unabto_env_base.h>

#include <unabto/unabto_external_environment.h>
#include <unabto/unabto_main_contexts.h>
#include <unabto/unabto_common_main.h>
#include <modules/log/unabto_basename.h>
#include <modules/log/syslog/unabto_syslog.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#ifndef WIN32
#include <sys/time.h>
#endif

#if defined(_MSC_VER)
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#endif

enum {
    SYSLOG_SEVERITY_EMERGENCY = 0,
    SYSLOG_SEVERITY_ALERT,
    SYSLOG_SEVERITY_CRITICAL,
    SYSLOG_SEVERITY_ERROR,
    SYSLOG_SEVERITY_WARNING,
    SYSLOG_SEVERITY_NOTICE,
    SYSLOG_SEVERITY_INFORMATION,
    SYSLOG_SEVERITY_DEBUG    
} syslog_severity_levels;

enum {
  SYSLOG_FACILITY_LOCAL0 = 16
} syslog_faclity;


enum {
    SYSLOG_MESSAGE_SIZE = 1500
};

/**
 * Set this to true when calling function which could potentially log to syslog,
 * such that no cyclig logging occurs.
 */
static bool disableSyslog = false; 

static nabto_socket_t syslogSocket;

int syslog_severity_from_severity(uint32_t severity);

bool unabto_syslog_init(const char* deviceId) 
{
    uint16_t localPort = 0;
    bool ok;
    disableSyslog = true;
    
    ok = nabto_init_socket(0, &localPort, &syslogSocket);
    disableSyslog = false;
    return ok;
}

size_t unabto_strnlen(const char *msg, size_t maxlen) {
#if _XOPEN_SOURCE >= 700 || _POSIX_C_SOURCE >= 200809L
    return strnlen(msg, maxlen);
#else    
    char *p = (char *)memchr(msg, 0, maxlen);
    return (size_t) (p ? (p-msg) : maxlen);
#endif
}

static bool unabto_create_syslog_header(char* syslogBuffer, size_t syslogBufferSize, uint32_t severity) 
{
    int syslogSeverity = syslog_severity_from_severity(severity);
    uint8_t syslogPriority = (SYSLOG_FACILITY_LOCAL0*8)+syslogSeverity;
    nabto_main_context* nmc = unabto_get_main_context();

#ifdef WIN32

    SYSTEMTIME st;
    GetSystemTime(&st);

    snprintf(syslogBuffer, syslogBufferSize, "<%u>1 %04u-%02u-%02uT%02u:%02u:%02u.%03uZ %s tunnel - - - ",
        syslogPriority, st.wYear, st.wMonth, st.wDay, 
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, nmc->nabtoMainSetup.id);
#else
    time_t sec;
    unsigned int us;
    struct timeval tv;
    struct tm tm;
    

    gettimeofday(&tv, NULL);
    sec = tv.tv_sec;
    us = tv.tv_usec;

    localtime_r(&sec, &tm);


    snprintf(syslogBuffer, syslogBufferSize, "<%u>1 %04u-%02u-%02uT%02u:%02u:%02u.%06uZ %s tunnel - - - ",
             syslogPriority, tm.tm_year, tm.tm_mon, tm.tm_mday,
             tm.tm_hour, tm.tm_min, tm.tm_sec, us, nmc->nabtoMainSetup.id);
#endif
    return true;
}

void unabto_syslog(uint32_t module, uint32_t severity, const char* file, unsigned int line, uint32_t syslogServer, uint16_t syslogPort, const char* format, va_list ap) 
{
    char msg[SYSLOG_MESSAGE_SIZE];
    size_t used;
    if (disableSyslog) {
        return;
    }

    memset(msg, 0, SYSLOG_MESSAGE_SIZE);
    used = 0;

    unabto_create_syslog_header(msg, SYSLOG_MESSAGE_SIZE, severity);
    used = unabto_strnlen(msg, SYSLOG_MESSAGE_SIZE);

    snprintf(msg+used, SYSLOG_MESSAGE_SIZE-used, "%s(%u): ", unabto_basename(file), line);
    used = unabto_strnlen(msg, SYSLOG_MESSAGE_SIZE);

    vsnprintf(msg+used, SYSLOG_MESSAGE_SIZE-used, format, ap);
    used = unabto_strnlen(msg,SYSLOG_MESSAGE_SIZE);

    disableSyslog = true;
    nabto_write(syslogSocket, (const uint8_t*)msg, used, syslogServer, syslogPort);
    disableSyslog = false;
}

void unabto_syslog_buffer(uint32_t module, uint32_t severity, const char* file, unsigned int line, uint32_t syslogServer, uint16_t syslogPort, const uint8_t* buffer, size_t buflen, const char* format, va_list ap) 
{
    size_t used;
    char msg[SYSLOG_MESSAGE_SIZE];
    size_t i, j;
    size_t linenums;
    
    if (disableSyslog) {
        return;
    }
    
    memset(msg, 0, SYSLOG_MESSAGE_SIZE);
    used = 0;

    unabto_create_syslog_header(msg, SYSLOG_MESSAGE_SIZE, severity);
    used = unabto_strnlen(msg, SYSLOG_MESSAGE_SIZE);

    snprintf(msg+used, SYSLOG_MESSAGE_SIZE-used, "%s(%u): ", unabto_basename(file), line);
    used = unabto_strnlen(msg, SYSLOG_MESSAGE_SIZE);

    vsnprintf(msg+used, SYSLOG_MESSAGE_SIZE-used, format, ap);
    used = unabto_strnlen(msg,SYSLOG_MESSAGE_SIZE);

    snprintf(msg+used, SYSLOG_MESSAGE_SIZE-used, "\n");
    used = unabto_strnlen(msg, SYSLOG_MESSAGE_SIZE);

    linenums = (buflen + 15) / 16;
    for (i = 0; i < linenums; i++) {
        snprintf(msg+used, SYSLOG_MESSAGE_SIZE-used, "%02x  ", (unsigned int) i * 16);
        used = unabto_strnlen(msg, SYSLOG_MESSAGE_SIZE);
        for (j = 0; j < 16 && i * 16 + j < buflen; j++) {
            snprintf(msg+used, SYSLOG_MESSAGE_SIZE-used, "%02x", buffer[i * 16 + j]);
            used = unabto_strnlen(msg, SYSLOG_MESSAGE_SIZE);
            if (j < 15) {
                snprintf(msg+used, SYSLOG_MESSAGE_SIZE-used, " ");
                used = unabto_strnlen(msg, SYSLOG_MESSAGE_SIZE);
            }
        }
        snprintf(msg+used, SYSLOG_MESSAGE_SIZE-used, "\n");
        used = unabto_strnlen(msg, SYSLOG_MESSAGE_SIZE);
    }

    disableSyslog = true;
    nabto_write(syslogSocket, (const uint8_t*)msg, used, syslogServer, syslogPort);
    disableSyslog = false;
}

int syslog_severity_from_severity(uint32_t severity) {
    switch (severity) {
        case NABTO_LOG_SEVERITY_FATAL:      return SYSLOG_SEVERITY_EMERGENCY;
        case NABTO_LOG_SEVERITY_ERROR:      return SYSLOG_SEVERITY_ERROR;
        case NABTO_LOG_SEVERITY_WARN:       return SYSLOG_SEVERITY_WARNING;
        case NABTO_LOG_SEVERITY_INFO:       return SYSLOG_SEVERITY_INFORMATION;
        case NABTO_LOG_SEVERITY_DEBUG:      return SYSLOG_SEVERITY_DEBUG;
        case NABTO_LOG_SEVERITY_TRACE:      return SYSLOG_SEVERITY_DEBUG;
        case NABTO_LOG_SEVERITY_BUFFERS:    return SYSLOG_SEVERITY_DEBUG; 
        case NABTO_LOG_SEVERITY_USER1:      return SYSLOG_SEVERITY_DEBUG;
        case NABTO_LOG_SEVERITY_STATISTICS: return SYSLOG_SEVERITY_DEBUG;
        case NABTO_LOG_SEVERITY_STATE:      return SYSLOG_SEVERITY_DEBUG;
        default: return SYSLOG_SEVERITY_DEBUG;
    }
}
