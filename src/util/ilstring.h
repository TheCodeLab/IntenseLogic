#ifndef ILU_STRING_H
#define ILU_STRING_H

#include <stdlib.h>

typedef struct il_string {
    size_t length, capacity, canary;
    char *data, *start;
    int *refs;
} il_string;

const il_string *il_string_static(const char *s);
il_string *il_string_new(const char *s, int len);
il_string *il_string_copy(const il_string *s);
char *il_string_cstring(const il_string *s, size_t *len);
int il_string_verify(const il_string *s);
il_string *il_string_ref(void* s);
void il_string_unref(void* s);
il_string *il_string_sub(il_string *s, int p1, int p2);
int il_string_cmp(const il_string *a, const il_string *b);
int il_string_byte(const il_string *s, int pos);
il_string *il_string_format(const char *fmt, ...);

#define il_l il_string_static
#define il_StoC(s) il_string_cstring(s, NULL)

#endif

