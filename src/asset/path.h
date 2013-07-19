#ifndef ILA_PATH_H
#define ILA_PATH_H

#include "util/ilstring.h"
#include "util/array.h"

typedef struct ilA_path {
    il_string *path;
    IL_ARRAY(il_string*,) nodes;
    int absolute;
} ilA_path;

ilA_path* ilA_path_string(il_string *path);
ilA_path* ilA_path_chars(const char *path);
ilA_path* ilA_path_cwd();
ilA_path* ilA_path_absolute(ilA_path *path);
ilA_path* ilA_path_copy(const ilA_path *self);
void ilA_path_free(ilA_path* self);
il_string *ilA_path_tostr(const ilA_path* self);
char *ilA_path_tochars(const ilA_path* self);
int ilA_path_cmp(const ilA_path* a, const ilA_path* b);
ilA_path* ilA_path_concat(const ilA_path* a, const ilA_path* b);
ilA_path* ilA_path_relativeTo(const ilA_path* a, const ilA_path* b);

#endif

