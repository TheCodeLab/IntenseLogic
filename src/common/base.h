#ifndef IL_BASE_H
#define IL_BASE_H

#include <stdlib.h>

#include "util/array.h"
#include "util/uthash.h"

#define IL_BASE_STARTUP   (0)
#define IL_BASE_TICK      (1)
#define IL_BASE_SHUTDOWN  (2)

#define IL_BASE_TICK_LENGTH (50000) // 50 000 microseconds (1/20 of a second)

typedef struct il_base_metadata {
    const char *key;
    void *value;
    UT_hash_handle hh;
} il_base_metadata;

typedef struct il_base il_base;
typedef struct il_type il_type;

typedef il_base *(*il_base_new_fn)(struct il_type*);
typedef il_base *(*il_base_copy_fn)(struct il_base*);
typedef void (*il_base_free_fn)(struct il_base*);

struct il_type {
    il_base_metadata* metadata;
    il_base_new_fn create;
    const char *name;
};

struct il_base {
    int refs;
    il_base_metadata *metadata;
    size_t size;
    il_base_free_fn destructor;
    il_base_copy_fn copy;
    il_base *gc_next;
    IL_ARRAY(il_base**,) weak_refs;
    il_type *type;
};

void *il_ref(void *obj);
void il_unref(void* obj);
void *il_metadata_get(void* obj, const char *key);
void *il_metadata_set(void *obj, const char *key, void *data);
void *il_type_metadata_get(void* obj, const char *key);
void *il_type_metadata_set(void *obj, const char *key, void *data);
size_t il_sizeof(void* obj);
il_type *il_typeof(void *obj);
il_base *il_new(il_type *type);
const char *il_name(il_type *type);

#endif

