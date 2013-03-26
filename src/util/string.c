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

const char *il_string_cstring(const il_string *s, size_t *len);
il_string *il_string_ref(void* s);
void il_string_unref(void* s);
il_string *il_string_sub(const il_string *s, int p1, int p2);
int il_string_cmp(const il_string *a, const il_string *b);
char il_string_byte(const il_string *s, int pos);
il_string *il_string_format(const char *fmt, ...);

