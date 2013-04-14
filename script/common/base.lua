local ffi = require "ffi"

ffi.cdef [[

typedef struct UT_hash_handle {
   struct UT_hash_table *tbl;
   void *prev;                       /* prev element in app order      */
   void *next;                       /* next element in app order      */
   struct UT_hash_handle *hh_prev;   /* previous hh in bucket order    */
   struct UT_hash_handle *hh_next;   /* next hh in bucket order        */
   void *key;                        /* ptr to enclosing struct's key  */
   unsigned keylen;                  /* enclosing struct's key len     */
   unsigned hashv;                   /* result of hash-fcn(key)        */
} UT_hash_handle;

enum il_metadatatype {
    IL_VOID,
    IL_STRING,
    IL_INT,
    IL_FLOAT,
    IL_METADATA,
    IL_OBJECT,
};

struct il_base_metadata;

typedef struct il_typeclass {
    const char *name; 
    UT_hash_handle hh;
} il_typeclass;

typedef struct il_base il_base;
typedef struct il_type il_type;

typedef il_base *(*il_base_new_fn)(struct il_type*);
typedef il_base *(*il_base_copy_fn)(struct il_base*);
typedef void (*il_base_free_fn)(struct il_base*);

struct il_type {
    il_typeclass *typeclasses;
    struct il_base_metadata* metadata;
    il_base_new_fn create;
    const char *name;
};

struct il_base {
    int refs;
    struct il_base_metadata *metadata;
    size_t size;
    il_base_free_fn destructor;
    il_base_copy_fn copy;
    il_base *gc_next;
    struct {
        il_base** data;
        size_t length;
        size_t capacity;
    } weak_refs;
    il_type *type;
};

void *il_ref(void *obj);
void il_unref(void* obj);
void *il_metadata_get(void *md, const char *key, size_t *size, enum il_metadatatype *tag);
void il_metadata_set(void *md, const char *key, void *data, size_t size, enum il_metadatatype tag);
size_t il_sizeof(void* obj);
il_type *il_typeof(void *obj);
il_base *il_new(il_type *type);
const char *il_name(il_type *type);
const void *il_cast(il_type* T, const char *to);
void il_impl(il_type* T, const char *name, void *impl);

]]

