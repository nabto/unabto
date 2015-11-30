#include <unabto/unabto_env_base.h>
#include <unabto/unabto_stream_event.h>
#include <unabto/unabto_stream_window.h>
#include <unabto/unabto_stream_environment.h>

enum {
    ACK     = NP_PAYLOAD_WINDOW_FLAG_ACK,
  //FAC     = NP_PAYLOAD_WINDOW_FLAG_FAC,
    FIN     = NP_PAYLOAD_WINDOW_FLAG_FIN,
    SYN     = NP_PAYLOAD_WINDOW_FLAG_SYN
};

// dummy stream accept impl
void unabto_stream_accept(unabto_stream* stream) {
    
}

void unabto_stream_event(unabto_stream* stream, unabto_stream_event_type event) {
    
}


bool test_state_machine_case(struct nabto_stream_s* stream, struct nabto_win_info* info, nabto_stream_tcb_state expectedState);

void make_syn_window(struct nabto_win_info* window) {
    memset(window, 0, sizeof(struct nabto_win_info));
    window->type = SYN;
}

void make_fin_ack_window(struct nabto_win_info* window, uint32_t ack, uint32_t seq) {
    memset(window, 0, sizeof(struct nabto_win_info));
    window->type = FIN | ACK;
    window->ack = ack;
    window->seq = seq;
}


void make_ack_window(struct nabto_win_info* window, uint32_t ack, uint32_t seq) {
    memset(window, 0, sizeof(struct nabto_win_info));
    window->type = ACK;
    window->ack = ack;
    window->seq = seq;
}

void resetStream(struct nabto_stream_s* stream)
{
    memset(stream, 0, sizeof(struct nabto_stream_s));
    x_buffer x_buffers[NABTO_STREAM_SEND_WINDOW_SIZE];
    r_buffer r_buffers[NABTO_STREAM_RECEIVE_WINDOW_SIZE];
    stream_initial_config(stream);
    stream_init_static_config(stream);
    stream->u.tcb.cfg.xmitWinSize = NABTO_STREAM_SEND_WINDOW_SIZE;
    stream->u.tcb.xmit = x_buffers;
    stream->u.tcb.recv = r_buffers;
}

bool test_state_machine(void) {
    bool ret = true;
    struct nabto_stream_s stream;

    resetStream(&stream);

    struct nabto_win_info window;
    make_syn_window(&window);

    stream.u.tcb.streamState = ST_IDLE;
    
    ret &= test_state_machine_case(&stream, &window, ST_SYN_RCVD);

    stream.u.tcb.streamState = ST_SYN_RCVD;
    ret &= test_state_machine_case(&stream, &window, ST_SYN_RCVD);

    NABTO_LOG_INFO(("Errors in the following lines is expected:"));

    int i;
    for (i = ST_ESTABLISHED; i <= ST_CLOSED; i++) {
        stream.u.tcb.streamState = i;
        ret &= test_state_machine_case(&stream, &window, i);
    }

    stream.u.tcb.streamState = ST_ESTABLISHED;
    stream.u.tcb.recvNext = 33;
    stream.u.tcb.xmitSeq = 42;

    // The fin has sequence number 33
    make_fin_ack_window(&window, 42, 33);
    ret &= test_state_machine_case(&stream, &window, ST_CLOSE_WAIT);
    
    stream.u.tcb.streamState = ST_LAST_ACK;
    // Assume a fin packet has been sent with sequence number 42.
    stream.u.tcb.finSequence = stream.u.tcb.xmitSeq;
    stream.u.tcb.xmitFirst = stream.u.tcb.xmitSeq;
    stream.u.tcb.xmitLastSent = stream.u.tcb.xmitFirst;
    stream.u.tcb.xmitLastSent++;
    stream.u.tcb.xmitSeq++;
    stream.u.tcb.ackTop = stream.u.tcb.xmitFirst;


    
    make_ack_window(&window, 43, 34);
    ret &= test_state_machine_case(&stream, &window, ST_CLOSED);    

    return ret;
}


bool test_state_machine_case(struct nabto_stream_s* stream, struct nabto_win_info* window, nabto_stream_tcb_state expectedState) {
    struct nabto_stream_sack_data sack;
    memset(&sack, 0, sizeof(struct nabto_stream_sack_data));
    nabto_stream_tcb_event(stream, window, 0, 0, &sack);
    
    if (stream->u.tcb.streamState != expectedState) {
        NABTO_LOG_ERROR(("Running state machine failed expected state %i but got state %i", expectedState, stream->u.tcb.streamState));
        return false;
    }
    return true;
}

