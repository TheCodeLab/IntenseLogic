#include "string.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

size_t strnlen(const char *s, size_t maxlen);

il_string il_CtoS(const char * s, int len)
{
    if (len < 0) {
        return (il_string) {strlen(s), s};
    }
    return (il_string) {strnlen(s, len)+1, s};
}

const char *il_StoC(il_string s)
{
    size_t len = strnlen(s.data, s.length);
    if (len < s.length) return s.data; // s already has a null terminator
    char * z = calloc(1, len+1);
    memcpy(z, s.data, len);
    z[len] = 0;
    return z;
}

il_string il_concatfunc(il_string s, ...)
{
    il_string str = (il_string) {0, NULL};
    va_list va;
    il_string arg = s;

    va_start(va, s);
    while (arg.length) {
        str.length+=strnlen(arg.data,arg.length);
        arg = va_arg(va, il_string);
    }
    va_end(va);
    str.data = (char*)calloc(1, str.length);
    va_start(va, s);
    char *p = (char*)str.data;
    arg = s;
    while (arg.length) {
        strncpy(p, arg.data, strnlen(arg.data,arg.length));
        p += strnlen(arg.data,arg.length);
        arg = va_arg(va, il_string);
    }
    va_end(va);

    return str;
}

int il_strcmp(il_String a, il_String b)
{
    size_t alen = strnlen(a.data,a.length);
    size_t blen = strnlen(b.data,b.length);
    if (alen < blen) return -1;
    return strncmp(a.data, b.data, blen);
}

size_t il_len(il_string s)
{
    return strnlen(s.data, s.length);
}

il_string il_strdup(il_string s)
{
    il_string s2 = s;
    s2.length = il_len(s);
    s2.data = calloc(s2.length, sizeof(char));
    strncpy((char*)s2.data, s.data, s2.length);
    return s2;
}

