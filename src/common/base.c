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

void *il_metadata_get(void* obj, const char *key)
{
    // HASH_FIND_STR (head, key_ptr, item_ptr)
    il_base_metadata *entry;
    HASH_FIND_STR(((il_base*)obj)->metadata, key, entry);
    if (entry) {
        return entry->value;
    }
    return NULL;
}

void *il_metadata_set(void *obj, const char *key, void *data)
{
    // HASH_REPLACE_STR (head,keyfield_name, item_ptr, replaced_item_ptr)
    il_base_metadata *entry;
    HASH_FIND_STR(((il_base*)obj)->metadata, key, entry);
    if (entry) {
        void *old = entry->value;
        entry->value = data;
        return old;
    }
    entry = calloc(1, sizeof(il_base_metadata));
    entry->key = key;
    entry->value = data;
    HASH_ADD_STR(((il_base*)obj)->metadata, key, entry);
    return NULL;
}

void *il_type_metadata_get(void* obj, const char *key)
{
    // HASH_FIND_STR (head, key_ptr, item_ptr)
    il_base_metadata *entry;
    HASH_FIND_STR(((il_type*)obj)->metadata, key, entry);
    if (entry) {
        return entry->value;
    }
    return NULL;
}

void *il_type_metadata_set(void *obj, const char *key, void *data)
{
    // HASH_REPLACE_STR (head,keyfield_name, item_ptr, replaced_item_ptr)
    il_base_metadata *entry;
    HASH_FIND_STR(((il_type*)obj)->metadata, key, entry);
    if (entry) {
        void *old = entry->value;
        entry->value = data;
        return old;
    }
    entry = calloc(1, sizeof(il_base_metadata));
    entry->key = key;
    entry->value = data;
    HASH_ADD_STR(((il_base*)obj)->metadata, key, entry);
    return NULL;
}

size_t il_sizeof(void* obj)
{
    size_t size = ((il_base*)obj)->size;
    if (size < sizeof(il_base)) {
        il_warning("Size of il_base<%p> is abnormal: %zu", obj, size);
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

