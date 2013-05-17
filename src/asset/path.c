#include "path.h"

#include <unistd.h>
#include <string.h>

#include "util/log.h"

char *strsep(char **stringp, const char *delim);
ilA_path* ilA_path_string(il_string *path)
{
    ilA_path* p;
    if (il_string_byte(path, 0) == '~') {
        char *home = getenv("HOME");
        if (home) {
            p = ilA_path_chars(home);
        } else {
            il_error("Attempted to parse string starting with ~ and no $HOME is set.");
            return NULL;
        }
    } else if (il_string_byte(path, 0) != '/') {
        p = ilA_path_cwd();
        if (!p) {
            return NULL;
        }
    } else {
        p = calloc(1, sizeof(ilA_path));
    }
    p->path = il_string_ref(path);
    char *orig = il_StoC(path), *str, *delim;
#ifdef WIN32
    delim = "\\/";
#else
    delim = "/";
#endif
    for (str = orig; str; strsep(&str, delim)) {
        if (strcmp(str, ".") == 0) {
            continue;
        } else if (strcmp(str, "..") == 0) {
            if (p->nodes.length < 1) {
                continue;
            }
            il_string_unref(p->nodes.data[p->nodes.length-1]);
            --p->nodes.length;
        } else {
            IL_APPEND(p->nodes, il_string_sub(path, str - orig, strlen(str)));
        }
    }
    free(orig);
    return p;
}

ilA_path* ilA_path_chars(const char *path)
{
    return ilA_path_string(il_string_new(path, strlen(path)+1));
}

ilA_path* ilA_path_copy(const ilA_path *self)
{
    ilA_path* p = calloc(1, sizeof(ilA_path));
    p->path = il_string_ref(self->path);
    unsigned i;
    for (i = 0; i < self->nodes.length; i++) {
        IL_APPEND(p->nodes, il_string_ref(self->nodes.data[i]));
    }
    return p;
}

ilA_path* ilA_path_cwd()
{
    char buf[4096];
    char *cwd = getcwd(buf, 4096);
    if (!cwd) {
        il_error("cwd too big");
        return NULL;
    }
    return ilA_path_chars(cwd);
}

void ilA_path_free(ilA_path* self)
{
    il_string_unref(self->path);
    unsigned i;
    for (i = 0; self->nodes.length; i++) {
        il_string_unref(self->nodes.data[i]);
    }
    IL_FREE(self->nodes);
}

il_string *ilA_path_tostr(const ilA_path* self)
{
    il_string *str = il_string_new(NULL, 0), *temp;
    unsigned i;
    for (i = 0; i < self->nodes.length; i++) {
        temp = self->nodes.data[i];
        il_string_cat(str, temp);
        if (i+1 < self->nodes.length) {
            il_string_catchars(str, "/");
        }
    }
    return str;
}

char *ilA_path_tochars(const ilA_path* self)
{
    il_string *s = ilA_path_tostr(self);
    char *buf = il_string_cstring(s, NULL);
    il_string_unref(s);
    return buf;
}

int ilA_path_cmp(const ilA_path* a, const ilA_path* b)
{
    unsigned i;
    il_string *as, *bs;
    unsigned min = a->nodes.length > b->nodes.length? b->nodes.length : a->nodes.length;
    for (i = 0; i < min; i++)  {
        as = a->nodes.data[i];
        bs = b->nodes.data[i];
        if (il_string_cmp(as, bs) != 0) {
            return 1;
        }
    }
    if (a->nodes.length < b->nodes.length) {
        return -1;
    }
    return 0;
}

ilA_path* ilA_path_concat(const ilA_path* a, const ilA_path* b)
{
    il_string *path = il_string_copy(a->path); 
    if (!il_string_cat(path, b->path)) {
        il_error("Failed to concatenate paths");
        return NULL;
    }
    ilA_path* p = ilA_path_string(path);
    il_string_unref(path);
    return p;
}

ilA_path* ilA_path_relativeTo(const ilA_path* a, const ilA_path* b)
{
    if (ilA_path_cmp(b,a) != -1) {
        return NULL;
    }
    ilA_path* p = calloc(1, sizeof(ilA_path));
    //p->nodes = mowgli_list_create();
    unsigned i;
    for (i = b->nodes.length; i < a->nodes.length; i++) {
        IL_APPEND(p->nodes, il_string_ref(a->nodes.data[i]));
    }
    p->path = ilA_path_tostr(p);
    return p;
}


