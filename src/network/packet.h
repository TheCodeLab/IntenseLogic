#ifndef ILN_PACKET_H
#define ILN_PACKET_H

#include <event2/util.h>
#include <stdint.h>

typedef struct ilN_packet {
    struct timeval arrival;
    uint16_t length;
    uint16_t seq;
    uint16_t ack;
    uint8_t flags;
    uint8_t channel;
    uint32_t object;
    uint32_t type;
    unsigned char data[];
} ilN_packet;

#endif

