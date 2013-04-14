#ifndef IL_BASE_H
#define IL_BASE_H

#include <stdlib.h>

#include "util/array.h"
#include "util/uthash.h"

#define IL_BASE_STARTUP   (0)
#define IL_BASE_TICK      (1)
#define IL_BASE_SHUTDOWN  (2)

#define IL_BASE_TICK_LENGTH (50000) // 50 000 microseconds (1/20 of a second)

enum il_storagetype {
    IL_VOID,
    IL_STRING,
    IL_INT,
    IL_FLOAT,
    IL_METADATA,
    IL_OBJECT,
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

typedef il_base *(*il_base_new_fn)(struct il_type*);
typedef il_base *(*il_base_copy_fn)(struct il_base*);
typedef void (*il_base_free_fn)(struct il_base*);

struct il_type {
    il_typeclass *typeclasses;
    il_storage* storage;
    il_base_new_fn create;
    const char *name;
};

struct il_base {
    int refs;
    il_storage* storage;
    size_t size;
    il_base_free_fn destructor;
    il_base_copy_fn copy;
    il_base *gc_next;
    IL_ARRAY(il_base**,) weak_refs;
    il_type *type;
};

void *il_ref(void *obj);
void il_unref(void* obj);
void *il_storage_get(void *md, const char *key, size_t *size, enum il_storagetype *tag);
void il_storage_set(void *md, const char *key, void *data, size_t size, enum il_storagetype tag);
size_t il_sizeof(void* obj);
il_type *il_typeof(void *obj);
il_base *il_new(il_type *type);
const char *il_name(il_type *type);
const void *il_cast(il_type* T, const char *to);
void il_impl(il_type* T, const char *name, void *impl);

#endif

