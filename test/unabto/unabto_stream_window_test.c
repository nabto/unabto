#include <unabto/unabto_stream_window.h>
#include <unabto/unabto_stream_types.h>

// init an rbuf as if it contains data
void init_rbuf(struct nabto_stream_tcb* tcb, uint32_t seq)
{
    uint16_t ix = seq % tcb->cfg.recvWinSize;
    r_buffer* rbuf = &tcb->recv[ix];
    rbuf->size = 42;
    rbuf->seq = seq;
}

bool unabto_stream_create_sack_pairs_test() {
    struct nabto_stream_s stream;
    

    struct nabto_stream_tcb* tcb = &stream.u.tcb;
    tcb->streamState = ST_ESTABLISHED;

    r_buffer rbufs[42];

    memset(rbufs,0,sizeof(rbufs));
    
    tcb->recv = rbufs;
    
    tcb->recvTop = 224; // upto and including 223 is cumulative filled.
    tcb->recvMax = 242;
    tcb->cfg.recvWinSize = 42;
    
    init_rbuf(tcb, 225);
    init_rbuf(tcb, 226);
    init_rbuf(tcb, 227);
    init_rbuf(tcb, 228);
    init_rbuf(tcb, 229);
    
    init_rbuf(tcb, 235);
    init_rbuf(tcb, 236);
    init_rbuf(tcb, 237);
    init_rbuf(tcb, 238);
    init_rbuf(tcb, 239);
    init_rbuf(tcb, 240);
    init_rbuf(tcb, 241);
    
    struct nabto_stream_sack_data sackData;
    if (!unabto_stream_create_sack_pairs(&stream, &sackData)) {
        return false;
    }

    if (sackData.nPairs != 2) {
        return false;
    }

    if (sackData.pairs[0].start != 235 || sackData.pairs[0].end != 242) {
        return false;
    }

    if (sackData.pairs[1].start != 225 || sackData.pairs[1].end != 230) {
        return false;
    }

    // there should now be
    // two sack areas 235..242 and 225..230

    return true;
    
}
