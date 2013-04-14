#include "base.h"

#include "util/log.h"

void *il_ref(void *obj)
{
    ((il_base*)obj)->refs++;
    return obj;
}

void il_unref(void* obj)
{
    if (--((il_base*)obj)->refs < 1) {
        ((il_base*)obj)->destructor(obj);
    }
}

void *il_metadata_get(void *md, const char *key, size_t *size, enum il_metadatatype *tag)
{
    // HASH_FIND_STR (head, key_ptr, item_ptr)
    il_metadata *entry;
    HASH_FIND_STR((il_metadata*)md, key, entry);
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

void il_metadata_set(void *md, const char *key, void *data, size_t size, enum il_metadatatype tag)
{
    void *buf;
    if (tag == IL_OBJECT) {
        buf = il_ref(data);
    } else {
        void *buf = malloc(size);
        memcpy(buf, data, size);
    }
    // HASH_REPLACE_STR (head,keyfield_name, item_ptr, replaced_item_ptr)
    il_metadata *entry;
    HASH_FIND_STR((il_metadata*)md, key, entry);
    if (entry) {
        void *old = entry->value;
        entry->value = buf;
        free(old);
        return;
    }
    entry = calloc(1, sizeof(il_metadata));
    entry->key = strdup(key);
    entry->value = buf;
    entry->tag = tag;
    switch(tag) {
        case IL_INT:
            entry->size = sizeof(int);
        case IL_FLOAT:
            entry->size = sizeof(float);
        case IL_METADATA:
            entry->size = sizeof(il_metadata);
        case IL_OBJECT:
            entry->size = il_sizeof(buf);
        case IL_VOID:
        case IL_STRING:
        default:
            entry->size = size;
    }
    il_metadata *obj = md;
    HASH_ADD_STR(obj, key, entry);
}

size_t il_sizeof(void* obj)
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

