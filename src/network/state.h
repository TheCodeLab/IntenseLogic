#ifndef ILN_STATE_H
#define ILN_STATE_H

#include <stdlib.h>
#include <stdint.h>
#include <event2/util.h>

#include "util/array.h"
#include "network/packet.h"

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
} ilN_state;

#endif

