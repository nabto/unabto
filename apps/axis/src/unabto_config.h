#ifndef UNABTO_CONFIG_H
#define UNABTO_CONFIG_H

#include <syslog.h>

#ifdef WIN32
#include <winsock2.h>
#else
#include <pthread.h>
#endif

#include <modules/log/dynamic/unabto_dynamic_log.h>

#define NABTO_CONNECTIONS_SIZE 5
#define NABTO_ENABLE_DEVICE_BUSY_AS_FATAL 1

#define NABTO_ENABLE_STREAM 1
#define NABTO_STREAM_MAX_STREAMS 30

#define NABTO_STREAM_RECEIVE_WINDOW_SIZE 100
#define NABTO_ENABLE_STATUS_CALLBACKS 0

#define NABTO_SET_TIME_FROM_ALIVE 0

#define NABTO_APPLICATION_EVENT_MODEL_ASYNC 1
#define NABTO_ENABLE_EXTENDED_RENDEZVOUS_MULTIPLE_SOCKETS 1

#define NABTO_APPREQ_QUEUE_SIZE NABTO_CONNECTIONS_SIZE
#define NABTO_ENABLE_DEBUG_PACKETS 1
#define NABTO_ENABLE_DEBUG_SYSLOG_CONFIG 1

#ifdef LOG_ALL
#define NABTO_LOG_ALL 1
#endif

#define NABTO_ENABLE_TCP_FALLBACK 1

/* #define CAT(x, y) CAT_I(x, y)  */
/* #define CAT_I(x, y) x ## y  */

/* #define APPLY(macro, args) APPLY_I(macro, args)  */
/* #define APPLY_I(macro, args) macro args  */

/* #define STRIP_PARENS(x) EVAL((STRIP_PARENS_I x), x)  */
/* #define STRIP_PARENS_I(...) 1,1  */

/* #define EVAL(test, x) EVAL_I(test, x)  */
/* #define EVAL_I(test, x) MAYBE_STRIP_PARENS(TEST_ARITY test, x)  */

/* #define TEST_ARITY(...) APPLY(TEST_ARITY_I, (__VA_ARGS__, 2, 1))  */
/* #define TEST_ARITY_I(a,b,c,...) c  */

/* #define MAYBE_STRIP_PARENS(cond, x) MAYBE_STRIP_PARENS_I(cond, x)  */
/* #define MAYBE_STRIP_PARENS_I(cond, x) CAT(MAYBE_STRIP_PARENS_, cond)(x)  */

/* #define MAYBE_STRIP_PARENS_1(x) x  */
/* #define MAYBE_STRIP_PARENS_2(x) APPLY(MAYBE_STRIP_PARENS_2_I, x)  */
/* #define MAYBE_STRIP_PARENS_2_I(...) __VA_ARGS__  */


/* #define NABTO_LOG_BASIC_PRINT(severity, cmsg) syslog(LOG_INFO, STRIP_PARENS(cmsg))  */

#endif
