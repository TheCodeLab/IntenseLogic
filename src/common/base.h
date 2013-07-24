/** @file base.h
 * @brief Object system for IL
 */

#ifndef IL_BASE_H
#define IL_BASE_H

#include <stdlib.h>

#include "util/array.h"
#include "util/uthash.h"
#include "common/event.h"

#define IL_BASE_TICK_LENGTH (50000) // 50 000 microseconds (1/20 of a second)

/** Enumeration of types supported by object storage */
enum il_storagetype {
    IL_VOID,                /**< C void* */
    IL_STRING,              /**< Null terminated C string */
    IL_INT,                 /**< C int type */
    IL_FLOAT,               /**< C float type */
    IL_STORAGE,             /**< Nested key-value storage */
    IL_OBJECT,              /**< The value has an il_base in it */
    IL_LUA,                 /**< The value is the result of luaL_ref() */
    IL_LOCAL_BIT = 0x70,    /**< second high bit determines whether or not to do network sync */
    IL_ARRAY_BIT = 0x80     /**< high bit is array signifier */
};

/** Arbitrary, nested, key-value data store associated with objects and types */
typedef struct il_storage {
    char *key;
    enum il_storagetype tag;
    size_t size;
    void *value;
    UT_hash_handle hh;
} il_storage;

/** Header that should be at the top of every typeclass */
#define il_typeclass_header const char *name; UT_hash_handle hh

/** Null typeclass */
typedef struct il_typeclass {
    il_typeclass_header;
} il_typeclass;

typedef struct il_base il_base;
typedef struct il_type il_type;

typedef void (*il_base_init_fn)(void *base);
typedef il_base *(*il_base_copy_fn)(void *base, const void *base2);
typedef void (*il_base_free_fn)(void *base);

/** Kind of like a class */
struct il_type {
    il_typeclass *typeclasses;      /** Hash table of trait implementations */
    il_storage *storage;            /** Storage for per-type data */
    il_base_init_fn constructor;    /** Called when il_new() and il_init() are called */
    il_base_free_fn destructor;     /** Called when il_unref() sees a zero-count object */
    il_base_copy_fn copy;           /** Called when il_copy() is invoked */
    const char *name;               /** The name of the type */
    ilE_registry *registry;         /** Registry of event handlers */
    size_t size;                    /** Size of the structure this type object represents */
    il_type *parent;                /** Parent type */
};

/** Header for instances of il_types */
struct il_base {
    int refs;                       /** Reference counter */
    il_base_free_fn free;           /** Function to free memory after object is no longer needed */
    il_storage *storage;            /** Key-value storage */
    il_base *gc_next;               /** Unused */
    IL_ARRAY(il_base**,) weak_refs; /** Weak references, see il_weakref() */
    il_type *type;                  /** Type object for this instance */
    ilE_registry *registry;         /** Registry of event handlers */
};

void *il_ref(void *obj);
void il_unref(void* obj);
/** Creates a weak reference. This will be set to NULL when the object is freed */
void il_weakref(void *obj, void **ptr);
void il_weakunref(void *obj, void **ptr);
/** Returns the data associated with a key, with optional parameters for data size/tag */
void *il_storage_get(il_storage **md, const char *key, size_t *size, enum il_storagetype *tag);
/** Sets the data associated with the given key. 
 *
 * The data will be copied unless tag is IL_VOID. 
 *
 * Size is used differently depending on the tag:
 * - For IL_VOID, it is the length of the data
 * - For IL_STRING, it is a parameter to strnlen()
 * - For anything with IL_ARRAY_BIT set, it is the number of elements
 * - For everything else, it is ignored
 */
void il_storage_set(il_storage **md, const char *key, void *data, size_t size, enum il_storagetype tag);
/** Wrapper for il_storage_get() */
void *il_type_get(il_type* self, const char *key, size_t *size, enum il_storagetype *tag);
/** Wrapper for il_storage_set() */
void il_type_set(il_type* self, const char *key, void *data, size_t size, enum il_storagetype tag);
/** Wrapper for il_storage_get() */
void *il_base_get(void* self, const char *key, size_t *size, enum il_storagetype *tag);
/** Wrapper for il_storage_set() */
void il_base_set(void* self, const char *key, void *data, size_t size, enum il_storagetype tag);
/** Returns the registry in the object, creating it if it doesn't exist */
ilE_registry *il_base_registry(il_base *self);
/** Returns the registry in the type, creating it if it doesn't exist */
ilE_registry *il_type_registry(il_type *self);
size_t il_sizeof(const il_type *self);
/** Returns the type object for any il_base */
il_type *il_typeof(void *obj);
/** Allocates a new object */
void *il_new(il_type *type);
/** Initializes a sufficiently sized region of memory to a new object */
void il_init(il_type *type, void *obj);
/** Creates a deep copy of an object */
void *il_copy(void *obj);
/** Returns a pointer to the name of a type, do not free it */
const char *il_name(il_type *type);
/** Looks up the typeclass implementation for the name to */
const void *il_cast(il_type* T, const char *to);
/** Registers an implementation for a typeclass */
void il_impl(il_type* T, void *impl);

#endif

