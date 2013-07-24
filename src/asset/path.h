/** @file path.h
 * @brief File path logic
 */

#ifndef ILA_PATH_H
#define ILA_PATH_H

#include "util/ilstring.h"
#include "util/array.h"

typedef struct ilA_path {
    il_string *path;
    IL_ARRAY(il_string*,) nodes;
    int absolute;
} ilA_path;

/** Creates a path from an il_string */
ilA_path* ilA_path_string(il_string *path);
/** Creates a path from a char* */
ilA_path* ilA_path_chars(const char *path);
/** Creates a path of the current working directory */
ilA_path* ilA_path_cwd();
/** Creates an absolute version of the specified path */
ilA_path* ilA_path_absolute(ilA_path *path);
/** Copies a path */
ilA_path* ilA_path_copy(const ilA_path *self);
/** Frees a path */
void ilA_path_free(ilA_path* self);
/** Converts a path to an il_string */
il_string *ilA_path_tostr(const ilA_path* self);
/** Converts a path to a char* */
char *ilA_path_tochars(const ilA_path* self);
/** Compares to paths, like strcmp(3) */
int ilA_path_cmp(const ilA_path* a, const ilA_path* b);
/** Concatenates two paths */
ilA_path* ilA_path_concat(const ilA_path* a, const ilA_path* b);
/** Chops b off of a 
 *
 * Example:
 *
 *      ilA_path_relativeTo("/usr/bin/foo", "/usr/bin") -> "/foo" 
 */
ilA_path* ilA_path_relativeTo(const ilA_path* a, const ilA_path* b);

#endif

