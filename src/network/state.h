#ifndef ILN_STATE_H
#define ILN_STATE_H

#include <stdlib.h>
#include <stdint.h>
#include <event2/util.h>

#include "util/array.h"
#include "network/packet.h"
#include "common/base.h"

typedef struct ilN_state {
    unsigned char received[65536/8];
    unsigned char sent[65536/8];
    uint16_t seq;
    evutil_socket_t fd;
    IL_ARRAY(ilN_packet*,) pending;
    IL_ARRAY(struct {
        uint16_t seq;
        struct timeval tv;
    },) unacknowledged;
    struct ilE_queue* channels[256];
    int kbps, kb_used;
    int total_sent, total_received;
    struct timeval ttl, timeout, last_msg;
    IL_ARRAY(il_base*,) objects;
    IL_ARRAY(il_type*,) types;
} ilN_state;

#define ilN_getbit(a, b) ((a[b/8]&(b%8))>>(b%8)) /*get the byte which contains 
                                                   b, & it so it only contains 
                                                   that bit, then shift that 
                                                   bit to the least significant
                                                   side */
#define ilN_setbit(a, b, v) (\
        a[b/8] &= ~(1<<(b%8)),     /*clear bit b of any value*/\
        a[b/8] |= ((v!=0)<<(b%8))) /*make sure v is either 1 or 0, other 
                                     values would screw it up*/

#endif

