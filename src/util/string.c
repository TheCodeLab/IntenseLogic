#include "ilstring.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

il_string il_string_static(char *s)
{
    return (il_string){s, strlen(s)};
}

il_string il_string_bin(char *s, size_t len)
{
    return (il_string){s, len};
}
