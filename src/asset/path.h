#ifndef ILA_PATH_H
#define ILA_PATH_H

#include <mowgli.h>

#include "util/ilstring.h"

typedef struct ilA_path {
    il_string *path;
    char *nodeptr;
    mowgli_list_t *nodes;
} ilA_path;

ilA_path* ilA_path_string(il_string *path);
ilA_path* ilA_path_chars(const char *path);
void ilA_path_free(ilA_path* self);
il_string *ilA_path_tostr(const ilA_path* self);
int ilA_path_cmp(const ilA_path* a, const ilA_path* b);
ilA_path* ilA_path_concat(const ilA_path* a, const ilA_path* b);
void ilA_path_relativeTo(ilA_path* self, const ilA_path* path);

#endif

