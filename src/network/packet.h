#ifndef ILN_PACKET_H
#define ILN_PACKET_H

#include <stdlib.h>
#include <event2/util.h>
#include <stdint.h>

enum ilN_packetflags {
    ILN_PACKET_SYNC      = 1,
    ILN_PACKET_RESET     = 2,
    ILN_PACKET_TYPE      = 4, // reserved by serializer, don't set it manually
    ILN_PACKET_OBJECT    = 8, // reserved by serializer, don't set it manually
};

typedef struct ilN_packet {
    struct timeval arrival;
    uint16_t seq;
    uint16_t ack;
    uint8_t flags;
    uint8_t channel;
    uint16_t event;
    uint32_t object;
    uint32_t type;
    uint16_t length;
    unsigned char *data;
} ilN_packet;

unsigned char *ilN_packet_serialize(const ilN_packet* self, size_t *size);
ilN_packet *ilN_packet_deserialize(const unsigned char *data, size_t size);

#endif

