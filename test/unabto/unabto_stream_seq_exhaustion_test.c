#include <unabto/unabto_env_base.h>
#include <unabto/unabto_connection.h>
#include <unabto/unabto_main_contexts.h>
#include <unabto/unabto_memory.h>
#include <unabto/unabto_stream.h>
#include <unabto/unabto_stream_event.h>
#include <unabto/unabto_stream_environment.h>
#include <unabto/unabto_stream_types.h>
#include <unabto/unabto_stream_window.h>

#include "unabto_stream_seq_exhaustion_test.h"

#include <string.h>

// unabto_stream_accept / unabto_stream_event dummies are defined in
// unabto_stream_event_test.c and resolved at link time.

enum {
    SEQ_TEST_ACK = NP_PAYLOAD_WINDOW_FLAG_ACK
};

static x_buffer seq_test_x[NABTO_STREAM_SEND_WINDOW_SIZE];
static r_buffer seq_test_r[NABTO_STREAM_RECEIVE_WINDOW_SIZE];
static struct nabto_connect_s seq_test_connection;

static void reset_seq_stream(struct nabto_stream_s* stream) {
    memset(stream, 0, sizeof(struct nabto_stream_s));
    memset(seq_test_x, 0, sizeof(seq_test_x));
    memset(seq_test_r, 0, sizeof(seq_test_r));
    memset(&seq_test_connection, 0, sizeof(seq_test_connection));
    stream->connection = &seq_test_connection;
    nmc.nabtoMainSetup.id = "";
    stream_initial_config(stream);
    stream_init_static_config(stream);
    stream->u.tcb.cfg = stream->u.tcb.initialConfig;
    stream->u.tcb.xmit = seq_test_x;
    stream->u.tcb.recv = seq_test_r;
    unabto_stream_congestion_control_init(&stream->u.tcb);
}

static bool test_send_seq_exhaustion(void) {
    struct nabto_stream_s* stream = &stream__[0];
    reset_seq_stream(stream);

    struct nabto_stream_tcb* tcb = &stream->u.tcb;
    tcb->streamState = ST_ESTABLISHED;
    tcb->xmitFirst = 0;
    tcb->xmitLastSent = 0;
    tcb->xmitSeq = 0;
    tcb->maxAdvertisedWindow = tcb->cfg.xmitWinSize;

    stream->staticConfig.seqExhaustionThreshold = 4;
    tcb->xmitDataCount = stream->staticConfig.seqExhaustionThreshold;

    uint8_t buf[8];
    memset(buf, 0xab, sizeof(buf));
    size_t queued = nabto_stream_tcb_write(stream, buf, sizeof(buf));

    if (queued != 0) {
        NABTO_LOG_ERROR(("seq exhaustion send: expected queued=0 after threshold, got %u", (unsigned)queued));
        return false;
    }
    if (!tcb->seqExhausted) {
        NABTO_LOG_ERROR(("seq exhaustion send: seqExhausted flag not set"));
        return false;
    }
    if (tcb->streamState != ST_CLOSED_ABORTED) {
        NABTO_LOG_ERROR(("seq exhaustion send: stream not force-closed (state=%d)", (int)tcb->streamState));
        return false;
    }

    unabto_stream_hint hint = UNABTO_STREAM_HINT_OK;
    size_t w = unabto_stream_write(stream, buf, sizeof(buf), &hint);
    if (w != 0) {
        NABTO_LOG_ERROR(("seq exhaustion send: unabto_stream_write returned %u, expected 0", (unsigned)w));
        return false;
    }
    if (hint != UNABTO_STREAM_HINT_SEQ_EXHAUSTED) {
        NABTO_LOG_ERROR(("seq exhaustion send: unabto_stream_write hint=%d, expected UNABTO_STREAM_HINT_SEQ_EXHAUSTED", (int)hint));
        return false;
    }
    return true;
}

static bool test_recv_seq_exhaustion(void) {
    struct nabto_stream_s* stream = &stream__[0];
    reset_seq_stream(stream);

    struct nabto_stream_tcb* tcb = &stream->u.tcb;
    tcb->streamState = ST_ESTABLISHED;

    stream->staticConfig.seqExhaustionThreshold = 4;
    tcb->recvDataCount = stream->staticConfig.seqExhaustionThreshold;

    struct nabto_win_info win;
    memset(&win, 0, sizeof(win));
    win.type = SEQ_TEST_ACK;
    win.ack = tcb->xmitSeq;
    win.seq = tcb->recvNext;

    struct nabto_stream_sack_data sack;
    memset(&sack, 0, sizeof(sack));

    nabto_stream_tcb_event(stream, &win, NULL, 0, &sack);

    if (!tcb->seqExhausted) {
        NABTO_LOG_ERROR(("seq exhaustion recv: seqExhausted flag not set"));
        return false;
    }
    if (tcb->streamState != ST_CLOSED_ABORTED) {
        NABTO_LOG_ERROR(("seq exhaustion recv: stream not force-closed (state=%d)", (int)tcb->streamState));
        return false;
    }
    return true;
}

bool unabto_stream_seq_exhaustion_test(void) {
    bool ret = true;
    ret &= test_send_seq_exhaustion();
    ret &= test_recv_seq_exhaustion();
    return ret;
}
