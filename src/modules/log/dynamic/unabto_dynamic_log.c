#include "unabto_dynamic_log_util.h"
#include <unabto/unabto_env_base.h>
#include <unabto/unabto_logging.h>
#include "unabto_dynamic_log.h"
#include <modules/log/unix/unabto_logging_unix.h>
#include <platforms/unabto_printf_logger.h>
#include <unabto/unabto_context.h>
#include <unabto/unabto_external_environment.h>

#include <modules/log/syslog/unabto_syslog.h>
#include <modules/log/unabto_log_header.h>
#include <stdarg.h>
#include <string.h>

static bool check_syslog_state();

bool convert_pattern_to_module_and_severity(const char* pattern, size_t patternLength, uint32_t* module, uint32_t* severity) {
    const char* dotIndex = strchr(pattern, '.');
    const char* patternEnd = pattern+patternLength;
    const char* moduleStart;
    const char* moduleEnd;
    const char* severityStart;
    const char* severityEnd;
    
    if (dotIndex == NULL) {
        NABTO_LOG_ERROR(("No . in log pattern"));
        return false;
    }

    moduleStart = pattern;
    moduleEnd = dotIndex;
    
    severityStart = dotIndex+1;
    severityEnd = patternEnd;

    if (!convert_module(moduleStart, moduleEnd, module)) {
        return false;
    }
    
    if (!convert_severity(severityStart, severityEnd, severity)) {
        return false;
    }

    return true;
}

static uint32_t stdout_module = 0;
static uint32_t stdout_severity = 0;


static uint32_t syslog_module = 0;
static uint32_t syslog_severity = 0;
static uint32_t syslog_host = 0;
static uint16_t syslog_port = 0;
static uint32_t syslog_expire = 0;
static nabto_stamp_t syslog_expire_stamp;
static bool syslog_enabled = false;
static bool syslog_initialized = false;


void unabto_log_system_log(uint32_t module, uint32_t severity, const char* file, unsigned int line, const char* format, ...) {
    if ((module & stdout_module) && (severity & stdout_severity)) {
        va_list args;
        unabto_log_header(file, line);
        va_start (args, format);
        vprintf (format, args);
        va_end (args);
        printf("\n");
    }

    if (check_syslog_state() && (module & syslog_module) && (severity & syslog_severity)) {
        va_list args;
        va_start (args, format);
        unabto_syslog(module, severity, file, line, syslog_host, syslog_port, format, args);
        va_end (args);
    }
    return;
}


void unabto_log_system_log_buffer(uint32_t module, uint32_t severity, const char* file, unsigned int line, const uint8_t* buffer, size_t bufferLength, const char* format, ...) 
{
    if ((module & stdout_module) && (severity & stdout_severity)) {
        va_list args;

        unabto_log_header(file, line);
        va_start (args, format);
        vprintf (format, args);
        va_end (args);
        printf("\n");
        log_buffer(buffer, bufferLength);
    }

    if (check_syslog_state() && (module & syslog_module) && (severity & syslog_severity)) {
        va_list args;
        va_start (args, format);
        unabto_syslog_buffer(module, severity, file, line, syslog_host, syslog_port, buffer, bufferLength, format, args);
        va_end (args);
    }
}

bool check_syslog_state() {
    if (!syslog_enabled) {
        return false;
    }

    if (syslog_expire && nabtoIsStampPassed(&syslog_expire_stamp)) {
        unabto_log_system_disable_syslog();
        NABTO_LOG_INFO(("Disabling syslog since it has expired"));
    }

    return syslog_enabled;
}

bool unabto_log_system_enable_stdout_pattern(const char* pattern) {
    uint32_t module;
    uint32_t severity;
    if (!convert_pattern_to_module_and_severity(pattern, strlen(pattern), &module, &severity)) {
        return false;
    }
    
    return unabto_log_system_enable_stdout(module, severity);
}

bool unabto_log_system_enable_stdout(uint32_t module, uint32_t severity) {
    stdout_module |= module;
    stdout_severity |= severity;
    return true;
}

void unabto_log_system_disable_stdout() {
    stdout_module = 0;
    stdout_severity = 0;
}

bool unabto_log_system_enable_syslog_pattern(const char* pattern, size_t patternLength, uint32_t syslogHost, uint16_t syslogPort, uint32_t expire) {
    uint32_t module;
    uint32_t severity;

    if (!syslog_initialized) {
        unabto_syslog_init();
        syslog_initialized = true; 
    }

    if (!convert_pattern_to_module_and_severity(pattern, patternLength, &module, &severity)) {
        return false;
    }

    return unabto_log_system_enable_syslog(module, severity, syslogHost, syslogPort, expire);
}

bool unabto_log_system_enable_syslog(uint32_t module, uint32_t severity, uint32_t syslogHost, uint16_t syslogPort, uint32_t syslogExpire) {
    syslog_host = syslogHost;
    syslog_port = syslogPort;
    syslog_expire = syslogExpire;
    if (syslog_expire > 0) {
        nabtoSetFutureStamp(&syslog_expire_stamp, (syslog_expire*1000));
    }
    syslog_module |= module;
    syslog_severity |= severity;
    syslog_enabled = true;
    return true;
}


void unabto_log_system_disable_syslog() {
    syslog_enabled = false;
    syslog_host = 0;
    syslog_port = 0;
    syslog_expire = 0;
    syslog_module = 0;
    syslog_severity = 0;
}

bool unabto_debug_syslog_config(bool enableSyslog, uint8_t facility,  uint32_t ip, uint16_t port, uint32_t expire, const uint8_t* configStr, uint16_t configStrLength) 
{
    bool ret  = false;
    nabto_endpoint ep;
    ep.addr = ip; ep.port = port;

    NABTO_LOG_INFO(("Enabling syslog " PRIep " %.*s expire %" PRIu32 " enabled %i", MAKE_EP_PRINTABLE(ep), configStrLength, configStr, expire, enableSyslog));
    
    if (enableSyslog) {
        ret = unabto_log_system_enable_syslog_pattern((const char*)configStr, configStrLength, ip, port, expire);
    } else {
        unabto_log_system_disable_syslog(); 
    }
    return ret;
}
