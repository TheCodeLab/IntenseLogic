#include "base.h"

#include "util/log.h"
#include "util/ilassert.h"

char *strdup(const char*);
size_t strnlen(const char*,size_t);

void *il_ref(void *obj)
{
    il_return_null_on_fail(obj);
    ((il_base*)obj)->refs++;
    return obj;
}

void il_unref(void* obj)
{
    il_return_on_fail(obj);
    il_base *base = obj;
    if (!base) {
        return;
    }
    if (--base->refs < 1) {
        size_t i;
        for (i = 0; i < base->weak_refs.length; i++) {
            *base->weak_refs.data[i] = NULL; // clear all the weak references
        }
        il_type *cur = il_typeof(base);
        while (cur) {
            if (cur->destructor) {
                cur->destructor(obj);
            }
            cur = cur->parent;
        }
        if (base->free) {
            base->free(obj);
        } else {
            free(obj);
        }
    }
}

void il_weakref(void *obj, void **ptr)
{
    il_return_on_fail(obj && ptr);
    il_base *base = obj;
    IL_APPEND(base->weak_refs, (il_base**)ptr);
    *ptr = obj;
}

void il_weakunref(void *obj, void **ptr)
{
    il_return_on_fail(obj && ptr);
    il_base *base = obj;
    size_t i;
    for (i = 0; i < base->weak_refs.length; i++) {
        if (base->weak_refs.data[i] == (il_base**)ptr) {
            base->weak_refs.data[i] = base->weak_refs.data[base->weak_refs.length-1];
            base->weak_refs.length--;
            break;
        }
    }
    *ptr = NULL;
}

il_table *il_base_getStorage(void *obj)
{
    return &((il_base*)obj)->storage;
}

il_table *il_type_getStorage(il_type *T)
{
    return &T->storage;
}

size_t il_sizeof(const il_type* self)
{
    il_return_val_on_fail(self, 0);
    size_t size = self->size;
    if (size < sizeof(il_base)) {
        il_warning("Size of type %s<%p> is abnormally small: %zu bytes, should be at least %zu bytes", self->name, self, size, sizeof(il_base));
        size = sizeof(il_base);
    } else if (self->parent && size < self->parent->size) {
        il_warning("Size of type %s<%p> is abnormally small: %zu bytes, parent type is %zu bytes", self->name, self, size, self->parent->size);
        size = self->parent->size;
    }
    return size;
}

il_type *il_typeof(void *obj)
{
    il_return_null_on_fail(obj);
    il_type *T = ((il_base*) obj)->type;
    if (!T) {
        il_error("No type for object %p", obj);
    }
    return T;
}

void *il_new(il_type *type)
{
    il_return_null_on_fail(type);
    il_base *obj = calloc(1, il_sizeof(type));
    il_init(type, obj);
    return obj;
}

void il_init(il_type *type, void *obj)
{
    il_return_on_fail(type && obj);
    il_base *base = obj;
    base->refs = 1;
    base->type = type;
    il_type *cur = type;
    while(cur) {
        if (cur->constructor) {
            cur->constructor(base);
        }
        cur = cur->parent;
    }
}

void *il_copy(void *obj)
{
    il_return_null_on_fail(obj);
    il_type *cur = il_typeof(obj);
    il_base *new = calloc(1, il_sizeof(cur));
    new->refs = 1;
    new->type = cur;
    while (cur) {
        if (cur->copy) {
            cur->copy(new, obj);
        }
        cur = cur->parent;
    }
    return new;
}

const char *il_name(il_type *type)
{
    il_return_null_on_fail(type);
    return type->name;
}

const void *il_cast(il_type* T, const char *to)
{
    il_return_null_on_fail(T);
    il_return_null_on_fail(to);
    il_typeclass *tc;
    HASH_FIND_STR(T->typeclasses, to, tc);
    return tc;
}

void il_impl(il_type* T, void *impl)
{
    il_return_on_fail(T && impl);
    il_typeclass *tc = impl;
    HASH_ADD_KEYPTR(hh, T->typeclasses, tc->name, strlen(tc->name), tc);
}

