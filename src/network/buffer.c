#include "buffer.h"

#include <stdlib.h>
#include "util/array.h"
#include "util/log.h"

struct ilN_buf {
    IL_ARRAY(unsigned char,) buf;
    int byte, bit;
};

ilN_buf* ilN_buf_new()
{
    ilN_buf *buf = calloc(1, sizeof(ilN_buf));
    return buf;
}

void ilN_buf_free(ilN_buf* self)
{
    IL_FREE(self->buf);
    free(self);
}

void ilN_buf_in(ilN_buf* self, const void *data, size_t size)
{
    ilN_buf_align(self);
    size_t old = self->buf.capacity;
    while (size + old > self->buf.capacity) {
        IL_RESIZE(self->buf);
    }
    memcpy(self->buf.data + self->byte, data, size);
    //self->byte += size;
    self->buf.length += size;
}

unsigned char *ilN_buf_out(ilN_buf* self, size_t *size)
{
    unsigned char *buf = malloc(self->buf.length);
    memcpy(buf, self->buf.data, self->buf.length);
    if (size) {
        *size = self->buf.length;
    }
    return buf;
}

void ilN_buf_clear(ilN_buf* self)
{
    self->buf.length = 0;
    self->byte = 0;
    self->bit = 0;
}

void ilN_buf_align(ilN_buf* self)
{
    if (self->bit != 0) {
        self->bit = 0;
        self->byte++;
    }
}

int ilN_buf_tell(ilN_buf *self)
{
    return self->byte*8 + self->bit;
}

int ilN_buf_seek(ilN_buf *self, unsigned int pos)
{
    self->byte = pos/8;
    self->bit = pos%8;
    if (self->byte >= self->buf.length) {
       return 0;
    }
    return 1; 
}

// writes val to the current byte, up to bits, returning the number written
static int write_bits(ilN_buf* self, int val, int bits) // assumes 8-bit bytes, horrible right
{
    unsigned char byte = val;
    byte &= (1<<bits)-1;
    byte <<= self->bit;
    self->buf.data[self->byte] |= byte;
    int n = 8-self->bit;
    self->bit = self->bit + bits;
    int bump = self->bit > 7;
    if (bump) {
        self->byte++;
        self->bit = 0;
    }
    return n;
}

void ilN_buf_writei(ilN_buf* self, int64_t i, int size)
{
    ilN_buf_writeu(self, (uint64_t)i, size); // that's right, I assume two's complement as well. deal with it.
}

void ilN_buf_writeu(ilN_buf* self, uint64_t i, int size)
{
    int bit = 0;
    while (bit < size) {
        bit += write_bits(self, i>>bit, 64-bit);
    }
}

void ilN_buf_writef(ilN_buf* self, float f)
{
    ilN_buf_writeu(self, *(uint32_t*)&f, 32); // assuming IEEE 754 too, we've got a badass in the building
}

void ilN_buf_writed(ilN_buf* self, double d)
{
    ilN_buf_writeu(self, *(uint64_t*)&d, 64);
}

void ilN_buf_writes(ilN_buf* self, const il_string *s)
{
    ilN_buf_in(self, s->data, s->length);
}

void ilN_buf_writec(ilN_buf* self, const char *s, size_t len)
{
    ilN_buf_in(self, s, strnlen(s, len));
}

static int ilN_buf_check(ilN_buf *self, size_t bytes)
{
    if (self->buf.length < bytes + self->byte) {
        il_error("Not enough space in buffer<%p> for operation: %zu bytes needed, %zu available", self, bytes, self->buf.length - self->byte);
        return 0;
    }
    return 1;
}

static int read_bits(ilN_buf *self, uint64_t *v, int off, int bits)
{
    int byte = self->buf.data[self->byte];
    byte >>= self->bit;
    byte &= (1<<bits)-1;
    byte <<= off;
    *v |= byte;
    int n = 8-self->bit;
    self->bit += bits>n? n : bits;
    if (self->bit > 7) {
        self->bit = 0;
        self->byte++;
    }
    return n;
}

int64_t ilN_buf_readi(ilN_buf* self, int size)
{
    return (int64_t)ilN_buf_readu(self, size);
}

uint64_t ilN_buf_readu(ilN_buf* self, int size)
{
    if (!ilN_buf_check(self, size/8)) {
        return 0;
    }
    int bit = 0;
    uint64_t val = 0;
    while (bit < size) {
        bit += read_bits(self, &val, bit, size-bit);
    }
    return val;
}

float ilN_buf_readf(ilN_buf* self)
{
    if (!ilN_buf_check(self, 4)) {
        return 0;
    }
    uint32_t i = ilN_buf_readu(self, 32);
    return *(float*)&i;
}

double ilN_buf_readd(ilN_buf* self)
{
    if (!ilN_buf_check(self, 8)) {
        return 0;
    }
    uint64_t i = ilN_buf_readu(self, 64);
    return *(double*)&i;
}

il_string* ilN_buf_reads(ilN_buf* self, size_t len)
{
    char *buf = ilN_buf_readc(self, len);
    il_string *s = il_string_new(buf, len);
    free(buf);
    return s;
}

char* ilN_buf_readc(ilN_buf* self, size_t len)
{
    if (!ilN_buf_check(self, len)) {
        return 0;
    }
    ilN_buf_align(self);
    char *buf = malloc(len + 1);
    memcpy(buf, self->buf.data + self->byte, len);
    return buf;
}

