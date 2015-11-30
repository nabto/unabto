#ifndef _UNABTO_LOG_SYSTEM_UTIL_H_
#define _UNABTO_LOG_SYSTEM_UTIL_H_

#include <unabto_platform_types.h>

bool convert_module(const char* moduleStart, const char* moduleEnd, uint32_t* module);
bool convert_severity(const char* severityStart, const char* severityEnd, uint32_t* severity);

#endif
