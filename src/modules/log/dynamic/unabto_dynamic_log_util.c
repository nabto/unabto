
#include "unabto_dynamic_log_util.h"
#include <unabto/unabto_logging_defines.h>

#include <string.h>

bool cmp_strings(const char* str1Start, const char* str1End, const char* str2)
{
    size_t str1len = str1End - str1Start;
    size_t str2len = strlen(str2);
    if (str1len != str2len) {
        return false;
    }
    
    return (strncmp(str1Start, str2, str1len) == 0);

}

bool convert_module(const char* moduleStart, const char* moduleEnd, uint32_t* module) 
{
    if (cmp_strings(moduleStart, moduleEnd, "none")) {
        *module = NABTO_LOG_MODULE_NONE;
        return true;
    }
    if (cmp_strings(moduleStart, moduleEnd, "all") || cmp_strings(moduleStart, moduleEnd, "*")) {
        *module = NABTO_LOG_MODULE_ALL;
        return true;
    }
    if (cmp_strings(moduleStart, moduleEnd, "default")) {
        *module = NABTO_LOG_MODULE_DEFAULT;
        return true;
    }
    if (cmp_strings(moduleStart, moduleEnd, "unabto")) {
        *module = NABTO_LOG_MODULE_UNABTO;
        return true;
    }
    if (cmp_strings(moduleStart, moduleEnd, "unabto_application")) {
        *module = NABTO_LOG_MODULE_UNABTO_APPLICATION;
        return true;
    }
    if (cmp_strings(moduleStart, moduleEnd, "encryption")) {
        *module = NABTO_LOG_MODULE_ENCRYPTION;
        return true;
    }
    if (cmp_strings(moduleStart, moduleEnd, "stream")) {
        *module = NABTO_LOG_MODULE_STREAM;
        return true;
    }
    if (cmp_strings(moduleStart, moduleEnd, "access_control")) {
        *module = NABTO_LOG_MODULE_ACCESS_CONTROL;
        return true;
    }
    if (cmp_strings(moduleStart, moduleEnd, "platform")) {
        *module = NABTO_LOG_MODULE_PLATFORM;
        return true;
    }
    if (cmp_strings(moduleStart, moduleEnd, "memory")) {
        *module = NABTO_LOG_MODULE_MEMORY;
        return true;
    }
    if (cmp_strings(moduleStart, moduleEnd, "network")) {
        *module = NABTO_LOG_MODULE_NETWORK;
        return true;
    }
    if (cmp_strings(moduleStart, moduleEnd, "application")) {
        *module = NABTO_LOG_MODULE_APPLICATION;
        return true;
    }
    if (cmp_strings(moduleStart, moduleEnd, "stack")) {
        *module = NABTO_LOG_MODULE_STACK;
        return true;
    }
    if (cmp_strings(moduleStart, moduleEnd, "nslp")) {
        *module = NABTO_LOG_MODULE_NSLP;
        return true;
    }
    if (cmp_strings(moduleStart, moduleEnd, "peripheral")) {
        *module = NABTO_LOG_MODULE_PERIPHERAL;
        return true;
    }
    if (cmp_strings(moduleStart, moduleEnd, "device_driver")) {
        *module = NABTO_LOG_MODULE_DEVICE_DRIVER;
        return true;
    }
    if (cmp_strings(moduleStart, moduleEnd, "storage")) {
        *module = NABTO_LOG_MODULE_STORAGE;
        return true;
    }
    if (cmp_strings(moduleStart, moduleEnd, "serial_port")) {
        *module = NABTO_LOG_MODULE_SERIAL_PORT;
        return true;
    }
    if (cmp_strings(moduleStart, moduleEnd, "modbus")) {
        *module = NABTO_LOG_MODULE_MODBUS;
        return true;
    }
    if (cmp_strings(moduleStart, moduleEnd, "test")) {
        *module = NABTO_LOG_MODULE_TEST;
        return true;
    }

    return false;
}

bool convert_severity(const char* severityStart, const char* severityEnd, uint32_t* severity)
{
    if (cmp_strings(severityStart, severityEnd, "none")) {
        *severity = NABTO_LOG_SEVERITY_LEVEL_NONE;
        return true;
    }
    if (cmp_strings(severityStart, severityEnd, "all")) {
        *severity = NABTO_LOG_SEVERITY_ALL;
        return true;
    }
    if (cmp_strings(severityStart, severityEnd, "fatal")) {
        *severity = NABTO_LOG_SEVERITY_LEVEL_FATAL;
        return true;
    }
    if (cmp_strings(severityStart, severityEnd, "error")) {
        *severity = NABTO_LOG_SEVERITY_LEVEL_ERROR;
        return true;
    }
    if (cmp_strings(severityStart, severityEnd, "warn")) {
        *severity = NABTO_LOG_SEVERITY_LEVEL_WARN;
        return true;
    }
    if (cmp_strings(severityStart, severityEnd, "info")) {
        *severity = NABTO_LOG_SEVERITY_LEVEL_INFO;
        return true;
    }
    if (cmp_strings(severityStart, severityEnd, "debug")) {
        *severity = NABTO_LOG_SEVERITY_LEVEL_DEBUG;
        return true;
    }
    if (cmp_strings(severityStart, severityEnd, "trace")) {
        *severity = NABTO_LOG_SEVERITY_LEVEL_TRACE;
        return true;
    }
    if (cmp_strings(severityStart, severityEnd, "buffers")) {
        *severity = NABTO_LOG_SEVERITY_BUFFERS;
        return true;
    }
    if (cmp_strings(severityStart, severityEnd, "user1")) {
        *severity = NABTO_LOG_SEVERITY_USER1;
        return true;
    }
    if (cmp_strings(severityStart, severityEnd, "statistics")) {
        *severity = NABTO_LOG_SEVERITY_STATISTICS;
        return true;
    }
    if (cmp_strings(severityStart, severityEnd, "state")) {
        *severity = NABTO_LOG_SEVERITY_STATE;
        return true;
    }
    return false;
}
