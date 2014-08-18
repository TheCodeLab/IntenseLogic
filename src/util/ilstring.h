/** @file ilstring.h
 * @brief Strings
 */

#ifndef ILU_STRING_H
#define ILU_STRING_H

#include <stdlib.h>

typedef struct il_string {
    char *str;
    size_t len;
} il_string;

il_string il_string_new(char *s);
il_string il_string_bin(char *s, size_t len);

#define il_l il_string_new

#endif
