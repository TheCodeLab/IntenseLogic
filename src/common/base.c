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
        int i;
        for (i = 0; i < base->weak_refs.length; i++) {
            *base->weak_refs.data[i] = NULL; // clear all the weak references
        }
        ((il_base*)obj)->destructor(obj);
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
    int i;
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
    // HASH_FIND_STR (head, key_ptr, item_ptr)
    il_storage *entry;
    HASH_FIND_STR(*md, key, entry);
    if (entry) {
        /*printf("get: %p@%zu, tag %u, name %s\n", entry->value, entry->size, entry->tag, key);
        if (entry->tag == IL_STRING) {
            printf("contents: %s\n", entry->value);
        }*/
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
    switch(tag) {
        case IL_INT:
            size = sizeof(int);
            break;
        case IL_FLOAT:
            size = sizeof(float);
            break;
        case IL_STORAGE:
            size = sizeof(il_storage);
            break;
        case IL_OBJECT:
            size = il_sizeof(data);
            break;
        case IL_STRING:
            size = strnlen(data, size);
            break;
        case IL_VOID:
        default:
            break;
    }
    void *buf;
    if (tag == IL_OBJECT) {
        buf = il_ref(data);
    } else {
        buf = malloc(size);
        memcpy(buf, data, size);
    }
    /*printf("set: %p@%zu tag %u key %s\n", buf, size, tag, key);
    if (tag == IL_STRING) {
        printf("contents: %s\n", buf);
    }*/
    // HASH_REPLACE_STR (head,keyfield_name, item_ptr, replaced_item_ptr)
    il_storage *entry;
    HASH_FIND_STR(*md, key, entry);
    if (entry) {
        void *old = entry->value;
        if (entry->tag == IL_OBJECT) {
            il_unref(entry->value);
        } else {
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

size_t il_sizeof(const void* obj)
{
    size_t size = ((il_base*)obj)->size;
    if (size < sizeof(il_base)) {
        il_warning("Size of %s<%p> is abnormal: %zu", ((il_base*)obj)->type->name, obj, size);
        size = sizeof(il_base);
    }
    return size;
}

il_type *il_typeof(void *obj)
{
    return ((il_base*) obj)->type;
}

il_base *il_new(il_type *type)
{
    return type->create(type);
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

void il_impl(il_type* T, const char *name, void *impl)
{
    il_typeclass *tc = impl;
    HASH_ADD_STR(T->typeclasses, name, tc);
}

