#include "node.h"

#include "util/log.h"

struct union_entry {
    const ilA_dir *iface;
    il_base *obj;
};

struct ilunion {
    il_base base;
    IL_ARRAY(struct union_entry,) dirs;
};

static void union_des(void* ptr)
{
    struct ilunion *self = ptr;
    unsigned i;
    for (i = 0; i < self->dirs.length; i++) {
        il_unref(self->dirs.data[i].obj);
    }
    IL_FREE(self->dirs);
}

static il_base *union_lookup(void* ptr, const ilA_path *path)
{
    struct ilunion *self = ptr;
    il_base *res = NULL;
    unsigned i;

    for (i = 0; i < self->dirs.length && !res; i++) {
        struct union_entry entry = self->dirs.data[i];
        res = ilA_lookup(entry.iface, entry.obj, path);
    }

    return res;
}

static void union_derp()
{
    il_error("Operation is not applicable for union directories.");
}

il_type ilA_union_type = {
    .typeclasses = NULL,
    .storage = NULL,
    .constructor = NULL,
    .destructor = union_des,
    .copy = NULL,
    .name = "il.asset.union",
    .registry = NULL,
    .size = sizeof(struct ilunion),
    .parent = NULL
};

ilA_dir ilA_union_dir = {
    .name = "il.asset.dir",
    .lookup = union_lookup,
    .create = (ilA_dir_create_fn)union_derp,
    .mkdir  = (ilA_dir_mkdir_fn) union_derp,
    .delete = (ilA_dir_delete_fn)union_derp
};

void ilA_union_init()
{
    il_impl(&ilA_union_type, &ilA_union_dir);
}

il_base *ilA_union(const ilA_dir *ai, const ilA_dir *bi, il_base *a, il_base *b, const ilA_dir **res)
{
    struct union_entry entry;
    struct ilunion *d;

    if (!ai) {
        ai = il_cast(il_typeof(a), "il.asset.dir");
    }
    if (!ai) {
        il_error("a is not a directory.");
        return NULL;
    }
    if (!bi) {
        bi = il_cast(il_typeof(b), "il.asset.dir");
        il_error("b is not a directory");
        return NULL;
    }
    if (a->type == &ilA_union_type) {
        il_ref(b);
        entry = (struct union_entry){bi, b};
        IL_APPEND(((struct ilunion*)a)->dirs, entry);
        return a;
    } else if (b->type == &ilA_union_type) {
        il_ref(a);
        entry = (struct union_entry){
            .iface = ai,
            .obj = b
        };
        IL_APPEND(((struct ilunion*)b)->dirs, entry);
        return b;
    } else {
        il_ref(a);
        il_ref(b);
    }
    d = il_new(&ilA_union_type);
    entry = (struct union_entry){ai, a};
    IL_APPEND(d->dirs, entry);
    entry = (struct union_entry){bi, b};
    IL_APPEND(d->dirs, entry);
    if (res) {
        *res = &ilA_union_dir;
    }

    return &d->base;
}

