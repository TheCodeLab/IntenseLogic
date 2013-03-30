#include "path.h"

#include <unistd.h>

#include "util/log.h"

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
    p->nodes = mowgli_list_create();
    char *s, *ptr, *end;
    size_t len, size;
    for (s = ptr = il_string_cstring(path, &size); s; s = strchr(s, '/'), end = strchr(s, '/')) {
        if (!end) {
            end = s + size;
        }
        len = end - s;
        if (strncmp(s, ".", len) == 0) {
            continue;
        } else if (strncmp(s, "..", len) == 0) {
            mowgli_node_delete(p->nodes->tail, p->nodes);
        } else {
            mowgli_node_add(il_string_sub(path, ptr - s, len), mowgli_node_create(), p->nodes);
        }
    }
    free(ptr);
    return p;
}

ilA_path* ilA_path_chars(const char *path)
{
    return ilA_path_string(il_string_new(path, strlen(path)+1));
}

ilA_path* ilA_path_copy(ilA_path *self)
{
    ilA_path* p = calloc(1, sizeof(ilA_path));
    p->path = il_string_ref(self->path);
    p->nodes = mowgli_list_create();
    mowgli_node_t *n;
    MOWGLI_LIST_FOREACH(n, self->nodes->head) {
        mowgli_node_add(il_string_ref(n->data), mowgli_node_create(), self->nodes);
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
    mowgli_node_t *n, *tn;
    MOWGLI_LIST_FOREACH_SAFE(n, tn, self->nodes->head) {
        il_string_unref(n->data);
        mowgli_node_delete(n, self->nodes);
        mowgli_node_free(n);
    }
    mowgli_list_free(self->nodes);
}

il_string *ilA_path_tostr(const ilA_path* self)
{
    il_string *str = il_string_new(NULL, 0), *temp;
    mowgli_node_t *n;
    MOWGLI_LIST_FOREACH(n, self->nodes->head) {
        temp = n->data;
        il_string_cat(str, temp);
        if (n->next) {
            il_string_catchars(str, "/");
        }
    }
    return str;
}

int ilA_path_cmp(const ilA_path* a, const ilA_path* b)
{
    mowgli_node_t *an, *bn = b->nodes->head;
    il_string *as, *bs;
    MOWGLI_LIST_FOREACH(an, a->nodes->head) {
        if (!bn) {
            break;
        }
        as = an->data;
        bs = bn->data;
        if (il_string_cmp(as, bs) != 0) {
            return 1;
        }
        bn = bn->next;
    }
    if (a->nodes->count < b->nodes->count) {
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
    p->nodes = mowgli_list_create();
    mowgli_node_t *n;
    MOWGLI_LIST_FOREACH(n, b->nodes->head) {} // skip the stuff we don't want
    MOWGLI_LIST_FOREACH(n, n) {
        mowgli_node_add(il_string_ref(n->data), mowgli_node_create(), p->nodes);
    }
    p->path = ilA_path_tostr(p);
    return p;
}


