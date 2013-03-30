#include "path.h"

ilA_path* ilA_path_string(il_string *path)
{
    ilA_path* p = calloc(1, sizeof(ilA_path));
    p->path = il_string_ref(path);
    p->nodes = mowgli_list_create();
    char *s, *end;
    size_t len, size;
    for (s = p->nodeptr = il_string_cstring(path, &size); s; s = strchr(s, '/'), end = strchr(s, '/')) {
        if (!end) {
            end = s + size;
        }
        len = end - s;
        if (strncmp(s, ".", len) == 0) {
            continue;
        } else if (strncmp(s, "..", len) == 0) {
            mowgli_node_delete(p->nodes->tail, p->nodes);
        } else {
            mowgli_node_add(s, mowgli_node_create(), p->nodes);
        }
    }
    return p;
}

ilA_path* ilA_path_chars(const char *path)
{
    return ilA_path_string(il_string_new(path, strlen(path)+1));
}

void ilA_path_free(ilA_path* self)
{
    il_string_unref(self->path);
    free(self->nodeptr);
    mowgli_node_t *n, *tn;
    MOWGLI_LIST_FOREACH_SAFE(n, tn, self->nodes->head) {
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
        char *start = n->data,
             *end = strchr(start, '/');
        if (!end) {
            end = self->nodeptr + self->path->length;
        }
        temp = il_string_new(start, end-start);
        il_string_cat(str, temp);
        il_string_unref(temp);
        if (n->next) {
            il_string_catchars(str, "/");
        }
    }
    return str;
}

int ilA_path_cmp(const ilA_path* a, const ilA_path* b);
ilA_path* ilA_path_concat(const ilA_path* a, const ilA_path* b);
void ilA_path_relativeTo(ilA_path* self, const ilA_path* path);


