#include "unabto_dynamic_log_util.h"
#include <unabto/unabto_env_base.h>
#include <unabto/unabto_logging.h>
#include "unabto_dynamic_log.h"
#include <modules/log/unix/unabto_logging_unix.h>
#include <platforms/unabto_printf_logger.h>
#include <unabto/unabto_context.h>
#include <unabto/unabto_external_environment.h>

#include <modules/log/unabto_log_header.h>
#include <stdarg.h>
#include <string.h>

bool convert_pattern_to_module_and_severity(const char* pattern, size_t patternLength, uint32_t* module, uint32_t* severity) {
    const char* dotIndex = strchr(pattern, '.');
    const char* patternEnd = pattern + patternLength;
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

    severityStart = dotIndex + 1;
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

void unabto_log_system_log(uint32_t module, uint32_t severity, const char* file, unsigned int line, const char* format, ...) {
    if ((module & stdout_module) && (severity & stdout_severity)) {
        va_list args;
        unabto_log_header(file, line);
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        printf("\n");
    }
    return;
}

void unabto_log_system_log_buffer(uint32_t module, uint32_t severity, const char* file, unsigned int line, const uint8_t* buffer, size_t bufferLength, const char* format, ...) {
    if ((module & stdout_module) && (severity & stdout_severity)) {
        va_list args;

        unabto_log_header(file, line);
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        printf("\n");
        log_buffer(buffer, bufferLength);
    }
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
