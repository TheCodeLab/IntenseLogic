#include "base.h"

#include "util/log.h"

void *il_ref(void *obj)
{
    ((il_base*)obj)->refs++;
    return obj;
}

void il_unref(void* obj)
{
    il_base *base = obj;
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
    il_base *base = obj;
    IL_APPEND(base->weak_refs, (il_base**)ptr);
    *ptr = obj;
}

void il_weakunref(void *obj, void **ptr)
{
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

void *il_storage_get(il_storage **md, const char *key, size_t *size, enum il_storagetype *tag)
{
    il_storage *entry;
    HASH_FIND_STR(*md, key, entry);
    if (entry) {
        if (size) {
            *size = entry->size;
        }
        if (tag) {
            *tag = entry->tag;
        }
        if (entry->tag == IL_OBJECT) {
            return il_ref(entry->value);
        }
        return entry->value;
    }
    return NULL;
}

void il_storage_set(il_storage **md, const char *key, void *data, size_t size, enum il_storagetype tag)
{
    int is_array = (tag & IL_ARRAY_BIT) == IL_ARRAY_BIT;
    size_t nmemb = is_array? size:1, i;
    void *buf = NULL, *old;
    il_storage *entry;

    tag &= ~(IL_ARRAY_BIT|IL_LOCAL_BIT);
    switch(tag) {
        case IL_INT:
        size = sizeof(int) * nmemb;
        break;
        case IL_FLOAT:
        size = sizeof(float) * nmemb;
        break;
        case IL_STORAGE:
        size = sizeof(il_storage);
        break;
        case IL_OBJECT:
        size = il_sizeof(data) * nmemb;
        if (is_array) {
            buf = calloc(sizeof(il_base*), nmemb);
            for (i = 0; i < nmemb; i++) {
                ((il_base**)buf)[i] = il_ref(((il_base**)data)[i]);
            }
        } else {
            buf = il_ref(data);
        }
        break;
        case IL_STRING:
        if (is_array) {
            size = nmemb * sizeof(char*);
            buf = calloc(sizeof(char*), nmemb);
            for (i = 0; i < nmemb; i++) {
                ((char**)buf)[i] = strdup(((char**)data)[i]);
            }
        } else {
            size = strnlen(data, size);
        }
        break;
        case IL_VOID:
        tag |= IL_LOCAL_BIT;
        default:
        break;
    }
    if (!buf) {
        buf = malloc(size);
        memcpy(buf, data, size);
    }
    HASH_FIND_STR(*md, key, entry);
    if (entry) {
        old = entry->value;
        switch((unsigned)entry->tag) { // enum warnings are no fun when ORing flags :(
            case IL_OBJECT | IL_ARRAY_BIT:
            for (i = 0; i < entry->size/sizeof(il_base*); i++) {
                il_unref(((il_base**)old)[i]);
            }
            break;
            case IL_OBJECT:
            il_unref(entry->value);
            break;
            case IL_STRING | IL_ARRAY_BIT:
            for (i = 0; i < entry->size/sizeof(char*); i++) {
                free(((char**)old)[i]);
            }
            break;
            default:
            free(old);
        }
    } else {
        entry = calloc(1, sizeof(il_storage));
    }
    entry->key = strdup(key);
    entry->value = buf;
    entry->tag = tag;
    entry->size = size;
    HASH_ADD_KEYPTR(hh, *md, key, strlen(key), entry);
}

void *il_type_get(il_type* self, const char *key, size_t *size, enum il_storagetype *tag)
{
    return il_storage_get(&self->storage, key, size, tag);
}

void il_type_set(il_type* self, const char *key, void *data, size_t size, enum il_storagetype tag)
{
    il_storage_set(&self->storage, key, data, size, tag);
}

void *il_base_get(il_base* self, const char *key, size_t *size, enum il_storagetype *tag)
{
    return il_storage_get(&self->storage, key, size, tag);
}

void il_base_set(il_base* self, const char *key, void *data, size_t size, enum il_storagetype tag)
{
    il_storage_set(&self->storage, key, data, size, tag);
}

ilE_registry *il_base_registry(il_base *self)
{
    if (!self->registry) {
        self->registry = ilE_registry_new();
    }
    return self->registry;
}

ilE_registry *il_type_registry(il_type *self)
{
    if (!self->registry) {
        self->registry = ilE_registry_new();
    }
    return self->registry;
}

size_t il_sizeof(const il_type* self)
{
    size_t size = self->size;
    if (size < sizeof(il_base)) {
        il_warning("Size of type %s is abnormally small: %zu bytes, should be at least %zu bytes", self->name, size, sizeof(il_base));
        size = sizeof(il_base);
    } else if (self->parent && size < self->parent->size) {
        il_warning("Size of type %s is abnormally small: %zu bytes, parent type is %zu bytes", self->name, size, self->parent->size);
        size = self->parent->size;
    }
    return size;
}

il_type *il_typeof(void *obj)
{
    return ((il_base*) obj)->type;
}

void *il_new(il_type *type)
{
    il_base *obj = calloc(1, il_sizeof(type));
    il_init(type, obj);
    return obj;
}

void il_init(il_type *type, void *obj)
{
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
    return type->name;
}

const void *il_cast(il_type* T, const char *to)
{
    il_typeclass *tc;
    HASH_FIND_STR(T->typeclasses, to, tc);
    return tc;
}

void il_impl(il_type* T, void *impl)
{
    il_typeclass *tc = impl;
    HASH_ADD_KEYPTR(hh, T->typeclasses, tc->name, strlen(tc->name), tc);
}

