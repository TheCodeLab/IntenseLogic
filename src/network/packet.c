#include "packet.h"

#include "network/buffer.h"

unsigned char *ilN_packet_serialize(const ilN_packet* self, size_t *size)
{
    ilN_buf *buf = ilN_buf_new();
    uint8_t flags = self->flags;
    if (self->object) {
        flags |= ILN_PACKET_OBJECT;
        flags &= ~ILN_PACKET_TYPE;
    } else if (self->type) {
        flags |= ILN_PACKET_TYPE;
        flags &= ~ILN_PACKET_OBJECT;
    }
    ilN_buf_writeu8(buf, flags);
    ilN_buf_writeu8(buf, self->channel);
    ilN_buf_writeu16(buf, self->event);
    if (self->object) {
        ilN_buf_writeu32(buf, self->object);
    } else if (self->type) {
        ilN_buf_writeu32(buf, self->type);
    }
    if (self->data) {
        ilN_buf_writec(buf, (const char*)self->data, self->length);
    }
    unsigned char *data = ilN_buf_out(buf, size);
    ilN_buf_free(buf);
    return data;
}

ilN_packet *ilN_packet_deserialize(const unsigned char *data, size_t size)
{
    ilN_packet *p = calloc(1, sizeof(ilN_packet));
    ilN_buf *buf = ilN_buf_new();
    ilN_buf_in(buf, data, size);
    p->flags = ilN_buf_readu8(buf);
    p->channel = ilN_buf_readu8(buf);
    p->event = ilN_buf_readu16(buf);
    if (p->flags & ILN_PACKET_OBJECT) {
        p->object = ilN_buf_readu32(buf);
    } else if (p->flags & ILN_PACKET_TYPE) {
        p->object = ilN_buf_readu32(buf);
    }
    int pos = ilN_buf_tell(buf);
    p->length = size - pos/8;
    p->data = (unsigned char*)ilN_buf_readc(buf, p->length);
    ilN_buf_free(buf);
    return p;
}

