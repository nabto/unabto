#ifndef _UNABTO_STREAM_TYPE_H_
#define _UNABTO_STREAM_TYPE_H_

#if NABTO_ENABLE_STREAM_STANDALONE
#include "unabto_streaming_types.h"
#else
#include "unabto_env_base.h"
#endif

#include <unabto/unabto_stream.h>
#include <unabto/unabto_stream_window.h>


#if NABTO_ENABLE_STREAM && NABTO_ENABLE_MICRO_STREAM

enum nabto_stream_state
{
    STREAM_IDLE = 0,
    STREAM_IN_USE = 1
};

typedef struct nabto_stream_static_config
{
    uint16_t defaultStreamTimeout;
    uint16_t minRetrans;
    uint16_t maxRetransmissionTime;
    
} nabto_stream_static_config;

/** Stream state */
struct nabto_stream_s {
    uint16_t                          streamTag;       /**< the tag                     */
    struct nabto_connect_s*           connection;      /**< the connection              */
    enum nabto_stream_state           state;           /**< the state of the entry      */
    uint16_t                          idCP;            /**< ID, client part             */
    uint16_t                          idSP;            /**< ID, serveer part            */
    nabto_stream_static_config        staticConfig;    /**< static configuration */
    struct {
        bool                          dataReady : 1;
        bool                          dataWritten : 1;
        bool                          readClosed : 1;
        bool                          writeClosed : 1;
        bool                          closed : 1;
    } applicationEvents;
    struct {
        bool                          streamEnded : 1;
    } statisticsEvents;
    struct unabto_stream_stats_s      stats;           /**< Stats for the stream        */

    union {
        struct nabto_stream_tcb       tcb;             /**< state for unreliable con     */
    } u;
};

#ifdef __cplusplus
extern "C" {
#endif

void unabto_stream_init_data_structure(struct nabto_stream_s* stream);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // NABTO_ENABLE_STREAM

#endif
