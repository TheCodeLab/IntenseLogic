#include "ilstring.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

il_string il_string_new(char *s)
{
    return (il_string){s, strlen(s)};
}

il_string il_string_bin(char *s, size_t len)
{
    return (il_string){s, len};
}

bool il_string_cmp(il_string a, il_string b)
{
    return a.len == b.len && strncmp(a.str, b.str, a.len) == 0;
}
