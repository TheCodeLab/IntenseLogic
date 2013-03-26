#include "ilstring.h"

#include <string.h>

static size_t compute_canary(const il_string *s)
{
    return s->length ^ s->capacity ^ (size_t)s->data;
}

const il_string *il_string_static(const char *s)
{
    il_string *str = calloc(1, sizeof(il_string));
    str->length = strlen(s);
    str->capacity = strlen(s);
    str->data = (char*)s; // :(
    str->canary = compute_canary(str);
    return str;
}

il_string *il_string_new(const char *s)
{
    il_string *str = calloc(1, sizeof(il_string));
    str->length = strlen(s);
    str->capacity = strlen(s);
    str->start = str->data = strdup(s);
    str->canary = compute_canary(str);
    str->refs = calloc(1, sizeof(int));
    *str->refs = 1;
    return str;
}

il_string *il_string_copy(const il_string *old)
{
    if (!il_string_verify(old)) {
        return NULL;
    }
    il_string *str = calloc(1, sizeof(il_string));
    str->length = old->length;
    str->capacity = old->capacity;
    char *buf = calloc(1, old->length+1);
    memcpy(buf, old->data, old->length);
    str->data = str->start = buf;
    str->canary = compute_canary(str);
    str->refs = calloc(1, sizeof(int));
    *str->refs = 1;
    return str;
}

char *il_string_cstring(const il_string *s, size_t *len)
{
    if (!il_string_verify(s)) {
        return NULL;
    }
    size_t size = strnlen(s->data, s->length);
    if (len) {
        *len = size;
    }
    int has_zero = size < s->length && s->data[size] == 0;
    char *buf = calloc(1, strnlen(s->data, s->length) + !has_zero);
    memcpy(buf, s->data, size);
    if (!has_zero) {
        buf[size] = 0;
    }
    return buf;
}

int il_string_verify(const il_string *s)
{
    return  s->canary == compute_canary(s) &&
            s->data >= s->start &&
            s->capacity >= s->length &&
            (!s->refs || *s->refs > 0);
}

il_string *il_string_ref(void* ptr)
{
    il_string *s = ptr;
    if (il_string_verify(s) && s->refs) {
        (*s->refs)++;
        return s;
    }
    return NULL;
}

void il_string_unref(void* ptr)
{
    il_string *s = ptr;
    if (il_string_verify(s) && s->refs) {
        (*s->refs)--;
    }
}

il_string *il_string_sub(il_string *s, int p1, int p2)
{
    if (!il_string_verify(s)) {
        return NULL;
    }
    if (p1 < 0) {
        p1 += s->length;
    }
    if (p2 < 0) {
        p2 += s->length;
    }
    if (p1 < 0 || p2 < 0 || (size_t)p1 > s->length || (size_t)p2 > s->length || p1 > p2) {
        return NULL;
    }
    il_string *str = calloc(1, sizeof(il_string));
    str->length = p2 - p1;
    str->capacity = s->capacity - p1;
    str->start = s->start;
    str->data = s->data + p1;
    str->canary = compute_canary(str);
    str->refs = s->refs;
    (*str->refs)++;
    return str;
}

int il_string_cmp(const il_string *a, const il_string *b)
{
    if (!il_string_verify(a) || !il_string_verify(b)) {
        return ~0; // ???
    }
    if (a == b) {
        return 0;
    } else if (a->data == b->data) {
        return a->length - b->length;
    }
    return strncmp(a->data, b->data, a->length < b->length? a->length : b->length);
}

int il_string_byte(const il_string *s, int pos)
{
    if (pos < 0) {
        pos += s->length;
    }
    if (pos < 0 || (size_t)pos >= s->length) {
        return -1;
    }
    return s->data[pos];
}

il_string *il_string_format(const char *fmt, ...);

