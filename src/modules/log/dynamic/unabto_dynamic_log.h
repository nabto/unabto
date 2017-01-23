#ifndef _UNABTO_LOG_SYSTEM_H_
#define _UNABTO_LOG_SYSTEM_H_

#include <unabto_platform_types.h>

/**
 * Clear all enabled log outputs and patterns;
 */
void unabto_log_system_clear();

bool convert_pattern_to_module_and_severity(const char* pattern, size_t patternLength, uint32_t* module, uint32_t* severity);

/**
 * Enable stdout logger.
 * @param module  The module to enable this module bits is or'ed with the existing bits
 * @param severity  The severity to log, the severity bits is or'ed with the other already defined logbits.
 */
bool unabto_log_system_enable_stdout_pattern(const char* pattern);
bool unabto_log_system_enable_stdout(uint32_t module, uint32_t severity);
void unabto_log_system_disable_stdout();

bool unabto_log_system_enable_syslog_pattern(const char* pattern, size_t patternLength, uint32_t syslogHost, uint16_t syslogPort, uint32_t expire);
bool unabto_log_system_enable_syslog(uint32_t module, uint32_t severity, uint32_t syslogHost, uint16_t syslogPort, uint32_t expire);
void unabto_log_system_disable_syslog();


void unabto_log_system_log(uint32_t module, uint32_t severity, const char* file, unsigned int line, const char* format, ...);
void unabto_log_system_log_buffer(uint32_t module, uint32_t severity, const char* file, unsigned int line, const uint8_t* buffer, size_t bufferLength, const char* format, ...);

#define CAT(x, y) CAT_I(x, y) 
#define CAT_I(x, y) x ## y 

#define APPLY(macro, args) APPLY_I(macro, args) 
#define APPLY_I(macro, args) macro args 

#define STRIP_PARENS(x) EVAL((STRIP_PARENS_I x), x) 
#define STRIP_PARENS_I(...) 1,1 

#define EVAL(test, x) EVAL_I(test, x) 
#define EVAL_I(test, x) MAYBE_STRIP_PARENS(TEST_ARITY test, x) 

#define TEST_ARITY(...) APPLY(TEST_ARITY_I, (__VA_ARGS__, 2, 1)) 
#define TEST_ARITY_I(a,b,c,...) c 

#define MAYBE_STRIP_PARENS(cond, x) MAYBE_STRIP_PARENS_I(cond, x) 
#define MAYBE_STRIP_PARENS_I(cond, x) CAT(MAYBE_STRIP_PARENS_, cond)(x) 

#define MAYBE_STRIP_PARENS_1(x) x 
#define MAYBE_STRIP_PARENS_2(x) APPLY(MAYBE_STRIP_PARENS_2_I, x) 
#define MAYBE_STRIP_PARENS_2_I(...) __VA_ARGS__ 

#ifndef NABTO_LOG_ALL
#define NABTO_LOG_ALL 1
#endif

#ifndef NABTO_LOG_FATAL
#define NABTO_LOG_FATAL(message)                                    do { printf message; printf("\n"); NABTO_FATAL_EXIT; } while(0)
#endif

#ifndef NABTO_LOG_BUFFER
#define NABTO_LOG_BUFFER(severity, message, buffer, length) unabto_log_system_log_buffer(NABTO_LOG_MODULE_CURRENT, severity, __FILE__, __LINE__, buffer, length, STRIP_PARENS(message))
#endif

#ifndef NABTO_LOG_BASIC_PRINT
#define NABTO_LOG_BASIC_PRINT(severity, cmsg) unabto_log_system_log(NABTO_LOG_MODULE_CURRENT, severity, __FILE__, __LINE__, STRIP_PARENS(cmsg)) 
#endif

#endif
