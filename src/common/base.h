#ifndef IL_BASE_H
#define IL_BASE_H

#include <stdlib.h>

#include "util/array.h"
#include "util/uthash.h"
#include "common/event.h"

#define IL_BASE_TICK_LENGTH (50000) // 50 000 microseconds (1/20 of a second)

enum il_storagetype {
    IL_VOID,
    IL_STRING,
    IL_INT,
    IL_FLOAT,
    IL_STORAGE,
    IL_OBJECT,
    IL_LUA,
};

typedef struct il_storage {
    char *key;
    enum il_storagetype tag;
    size_t size;
    void *value;
    UT_hash_handle hh;
} il_storage;

#define il_typeclass_header const char *name; UT_hash_handle hh

typedef struct il_typeclass {
    il_typeclass_header;
} il_typeclass;

typedef struct il_base il_base;
typedef struct il_type il_type;

typedef void (*il_base_init_fn)(void *base);
typedef il_base *(*il_base_copy_fn)(void *base);
typedef void (*il_base_free_fn)(void *base);

struct il_type {
    il_typeclass *typeclasses;
    il_storage *storage;
    il_base_init_fn constructor;
    il_base_free_fn destructor;
    il_base_copy_fn copy;
    const char *name;
    ilE_registry *registry;
    size_t size;
    il_type *parent;
};

struct il_base {
    int refs;
    il_base_free_fn free;
    il_storage *storage;
    il_base *gc_next;
    IL_ARRAY(il_base**,) weak_refs;
    il_type *type;
    ilE_registry *registry;
};

void *il_ref(void *obj);
void il_unref(void* obj);
void il_weakref(void *obj, void **ptr);
void il_weakunref(void *obj, void **ptr);
void *il_storage_get(il_storage **md, const char *key, size_t *size, enum il_storagetype *tag);
void il_storage_set(il_storage **md, const char *key, void *data, size_t size, enum il_storagetype tag);
void *il_type_get(il_type* self, const char *key, size_t *size, enum il_storagetype *tag);
void il_type_set(il_type* self, const char *key, void *data, size_t size, enum il_storagetype tag);
void *il_base_get(il_base* self, const char *key, size_t *size, enum il_storagetype *tag);
void il_base_set(il_base* self, const char *key, void *data, size_t size, enum il_storagetype tag);
ilE_registry *il_base_registry(il_base *self);
ilE_registry *il_type_registry(il_type *self);
size_t il_sizeof(const il_type *self);
il_type *il_typeof(void *obj);
void *il_new(il_type *type);
void il_init(il_type *type, void *obj);
const char *il_name(il_type *type);
const void *il_cast(il_type* T, const char *to);
void il_impl(il_type* T, void *impl);

#endif

