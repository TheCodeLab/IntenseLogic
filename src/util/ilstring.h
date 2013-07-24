/** @file ilstring.h
 * @brief Counted string implementation
 */

#ifndef ILU_STRING_H
#define ILU_STRING_H

#include <stdlib.h>

typedef struct il_string {
    size_t length, capacity, canary;
    char *data, *start;
    int *refs;
} il_string;

/** Creates an il_string on a statically allocated string, with no copy needed */
const il_string *il_string_static(const char *s);
/** Creates a new null-terminated string */
il_string *il_string_new(const char *s, int len);
/** Creates a new string, but ignores embedded null terminators */
il_string *il_string_bin(const void *s, size_t len);
/** Copies a string, including the memory used (this does not create a new reference) */
il_string *il_string_copy(const il_string *s);
/** Creates a null-terminated string using malloc() and returns it, the user must free it */
char *il_string_cstring(const il_string *s, size_t *len);
/** Resizes a string's capacity to allow for more efficient concatenations */
int /*success*/ il_string_resize(il_string *self, size_t size);
/** Verifies that a string has not been corrupted, by checking its canary value */
int il_string_verify(const il_string *s);
/** Increments the reference counter */
il_string *il_string_ref(void* s);
/** Decrements the reference counter, and frees memory if it is 0 */
void il_string_unref(void* s);
/** Returns a substring without copying */
il_string *il_string_sub(il_string *s, int p1, int p2);
/** Compares two strings, the same way as strcmp(3) */
int il_string_cmp(const il_string *a, const il_string *b);
/** Returns the byte at the specified position */
int il_string_byte(const il_string *s, int pos);
/** Wrapper around snprintf(3) */
il_string *il_string_format(const char *fmt, ...);
/** Concatenates a string onto a mutable string */
int /*success*/ il_string_cat(il_string *self, const il_string *str);
/** Concatenates a C string onto a mutable string */
int /*success*/ il_string_catchars(il_string *self, const char *chars);
/** Convienience function for literals */
#define il_l il_string_static
#define il_StoC(s) il_string_cstring(s, NULL)

#endif

