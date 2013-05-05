#ifndef ILN_BUFFER_H
#define ILN_BUFFER_H

#include <stdint.h>

#include "util/ilstring.h"

typedef struct ilN_buf ilN_buf;

ilN_buf* ilN_buf_new();
void ilN_buf_free(ilN_buf* self);
void ilN_buf_in(ilN_buf* self, const void *data, size_t size);
unsigned char *ilN_buf_out(ilN_buf* self, size_t *size);
void ilN_buf_clear(ilN_buf* self);
void ilN_buf_align(ilN_buf* self);
void ilN_buf_makeRoom(ilN_buf* self, size_t bytes);
int ilN_buf_tell(ilN_buf *self);
int ilN_buf_seek(ilN_buf *self, unsigned int pos);
void ilN_buf_writei(ilN_buf* self, int64_t i, int size);
#define ilN_buf_writei8(b, i)  ilN_buf_writei(b, i, 8)
#define ilN_buf_writei16(b, i) ilN_buf_writei(b, i, 16)
#define ilN_buf_writei32(b, i) ilN_buf_writei(b, i, 32)
#define ilN_buf_writei64(b, i) ilN_buf_writei(b, i, 64)
void ilN_buf_writeu(ilN_buf* self, uint64_t i, int size);
#define ilN_buf_writeu8(b, i)  ilN_buf_writeu(b, i, 8)
#define ilN_buf_writeu16(b, i) ilN_buf_writeu(b, i, 16)
#define ilN_buf_writeu32(b, i) ilN_buf_writeu(b, i, 32)
#define ilN_buf_writeu64(b, i) ilN_buf_writeu(b, i, 64)
void ilN_buf_writef(ilN_buf* self, float f);
void ilN_buf_writed(ilN_buf* self, double d);
void ilN_buf_writes(ilN_buf* self, const il_string *s);
void ilN_buf_writec(ilN_buf* self, const char *s, size_t len);
int64_t ilN_buf_readi(ilN_buf* self, int size);
#define ilN_buf_readi8(b)  (int8_t) ilN_buf_readi(b, 8)
#define ilN_buf_readi16(b) (int16_t)ilN_buf_readi(b, 16)
#define ilN_buf_readi32(b) (int32_t)ilN_buf_readi(b, 32)
#define ilN_buf_readi64(b) (int64_t)ilN_buf_readi(b, 64)
uint64_t ilN_buf_readu(ilN_buf* self, int size);
#define ilN_buf_readu8(b)  (uint8_t) ilN_buf_readu(b, 8)
#define ilN_buf_readu16(b) (uint16_t)ilN_buf_readu(b, 16)
#define ilN_buf_readu32(b) (uint32_t)ilN_buf_readu(b, 32)
#define ilN_buf_readu64(b) (uint64_t)ilN_buf_readu(b, 64)
float ilN_buf_readf(ilN_buf* self);
double ilN_buf_readd(ilN_buf* self);
il_string* ilN_buf_reads(ilN_buf* self, size_t len);
char* ilN_buf_readc(ilN_buf* self, size_t len);


#endif

