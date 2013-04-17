---
-- Provides the basis of the IL object system.
-- @author tiffany

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

enum il_storagetype {
    IL_VOID,
    IL_STRING,
    IL_INT,
    IL_FLOAT,
    IL_METADATA,
    IL_OBJECT,
};

struct il_storage;

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
    struct il_storage* storage;
    il_base_new_fn create;
    const char *name;
};

struct il_base {
    int refs;
    struct il_storage *storage;
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
void il_weakref(void *obj, void **ptr);
void il_weakunref(void *obj, void **ptr);
void *il_metadata_get(void *md, const char *key, size_t *size, enum il_metadatatype *tag);
void il_metadata_set(void *md, const char *key, void *data, size_t size, enum il_metadatatype tag);
size_t il_sizeof(void* obj);
il_type *il_typeof(void *obj);
il_base *il_new(il_type *type);
const char *il_name(il_type *type);
const void *il_cast(il_type* T, const char *to);
void il_impl(il_type* T, const char *name, void *impl);

]]

local base = {}

local per_class = {}

ffi.metatype("il_base", {
    __index = function(t,k)
        local pc = per_class[t.type.name]
        if pc and pc.__index then
            local res = pc.__index(t,k)
            if res then return res end
        end
        return base[k]
    end
})

--- Creates a new type.
-- Has a bit of fun with syntax. Doesn't actually take 2 arguments, it returns a function which takes the second argument. Example: 
--
--    local my_type = base.type "my_type" {
--        constructor = function(self, ...)
--            print("construction!")
--        end,
--        destructor = function(self)
--            print("destruction!")
--        end,
--        __index = function(t,k)
--            print("__index!")
--        end,
--        struct = "some_ffi_structure"
--    }
--
-- @tparam string name The name of the type
-- @tparam tab cons The construction table of the type
-- @treturn cdata The type object
function base.type(name)
    return function(cons)
        per_class[name] = cons
        local T = ffi.new("il_type")
        T.name = name
        T.create = function(...)
            local v = ffi.new(cons.struct or "il_base")
            ffi.cdata(v, ffi.C.il_unref)
            v.destructor = function(v)
                if cons.destructor then
                    cons.destructor(v)
                end
            end
            if cons.constructor then
                cons.constructor(v, ...)
            end
            return v
        end
        return T
    end
end

--- Returns the type object that corresponds to the given object
-- @tparam base v The value to get the type of
-- @treturn type The type object
function base.typeof(v)
    assert(ffi.istype("il_base*", v) or ffi.istype("il_base", v))
    return ffi.C.il_typeof(v)
end

--- Creates a new instance of a type
-- @tparam type T The type object
-- @treturn base
function base.create(T)
    assert(ffi.istype("il_type*", T) or ffi.istype("il_type", T))
    return ffi.C.il_new(T)
end

setmetatable(base, {__call=function(self,...) return base.create(...) end})

return base

