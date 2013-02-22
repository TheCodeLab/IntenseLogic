#ifdef WIN32

#include <stdlib.h>
#include <string.h>

// http://stackoverflow.com/a/12979321
char *strtok_r(char *str, const char *delim, char **nextp)
{
    char *ret;

    if (str == NULL) {
        str = *nextp;
    }

    str += strspn(str, delim);

    if (*str == '\0') {
        return NULL;
    }

    ret = str;
    str += strcspn(str, delim);

    if (*str) {
        *str++ = '\0';
    }

    *nextp = str;
    return ret;
}

size_t strnlen(const char *str, size_t maxlen) // ripped off from glibc, stripped down significantly
{
    const char *char_ptr, *end_ptr = str + maxlen;
    if (maxlen == 0)
        return 0;
    if (end_ptr < str)
        end_ptr = (const char *) ~0UL;
    for (char_ptr = str; ; ++char_ptr) {
        if (*char_ptr == '\0') {
            if (char_ptr > end_ptr)
                char_ptr = end_ptr;
            return char_ptr - str;
        }
    }
    return end_ptr - str;
}

char *strdup(const char* str)
{
    char *new = malloc(strlen(str) + 1);
    strcpy(new, str);
    return new;
}

#endif

