#include "node.h"

struct prefix {
    il_base base;
    ilA_path *path;
    const ilA_dir *iface;
    il_base *dir;
};

static void prefix_des(void *ptr)
{
    struct prefix *self = ptr;
    ilA_path_free(self->path);
    il_unref(self->dir);
}

static il_base *prefix_lookup(void *ptr, const ilA_path *path)
{
    struct prefix *self = ptr;
    ilA_path *p = ilA_path_relativeTo(path, self->path);
    if (!p) {
        return NULL;
    }
    return ilA_lookup(self->iface, self->dir, p);
}

// TODO: implement create, mkdir, and delete

il_type ilA_prefix_type = {
    .typeclasses = NULL,
    .storage = NULL,
    .constructor = NULL,
    .destructor = prefix_des,
    .copy = NULL,
    .name = "il.asset.prefix",
    .registry = NULL,
    .size = sizeof(struct prefix),
    .parent = NULL
};

ilA_dir ilA_prefix_dir = {
    .name = "il.asset.dir",
    .lookup = prefix_lookup
};

il_base *ilA_prefix(const ilA_dir *iface, il_base *dir, const ilA_path *path, const ilA_dir **res)
{
    struct prefix *d = il_new(&ilA_prefix_type);
    d->path = ilA_path_copy(path);
    d->dir = il_ref(dir);
    d->iface = iface;
    if (res) {
        *res = &ilA_prefix_dir;
    }
    return &d->base;
}


