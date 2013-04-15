#ifndef ILN_STATE_H
#define ILN_STATE_H

#include <stdlib.h>
#include <stdint.h>
#include <event2/util.h>

#include "util/array.h"
#include "network/packet.h"
#include "common/base.h"

typedef struct ilN_state {
    struct ilE_queue* channels[256];
    int kbps, kb_used;
    int total_sent, total_received;
    IL_ARRAY(il_base*,) objects;
    IL_ARRAY(il_type*,) types;
} ilN_state;

#endif

