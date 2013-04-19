---
-- Provides the basis of the IL object system.
-- See examples/item.lua and examples/itemTest.lua for usage examples.
-- @author tiffany

local ffi = require "ffi"

ffi.cdef [[

struct ilE_registry;

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
    IL_STORAGE,
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
    struct ilE_registry *registry;
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
    struct ilE_registry *registry;
};

void *il_ref(void *obj);
void il_unref(void* obj);
void il_weakref(void *obj, void **ptr);
void il_weakunref(void *obj, void **ptr);
void *il_storage_get(struct il_storage **md, const char *key, size_t *size, enum il_storagetype *tag);
void il_storage_set(struct il_storage **md, const char *key, void *data, size_t size, enum il_storagetype tag);
void *il_type_get(il_type* self, const char *key, size_t *size, enum il_storagetype *tag);
void il_type_set(il_type* self, const char *key, void *data, size_t size, enum il_storagetype tag);
void *il_base_get(il_base* self, const char *key, size_t *size, enum il_storagetype *tag);
void il_base_set(il_base* self, const char *key, void *data, size_t size, enum il_storagetype tag);
size_t il_sizeof(void* obj);
il_type *il_typeof(void *obj);
il_base *il_new(il_type *type);
const char *il_name(il_type *type);
const void *il_cast(il_type* T, const char *to);
void il_impl(il_type* T, const char *name, void *impl);

]]

local base = {}

local per_class = {}

local function metafun(name, def)
    return function(t, ...)
        local pc = per_class[ffi.string(t.type.name)]
        if pc and pc[name] then
            local res = pc[name](t, ...)
            if res then return res end
        end
        if def then
            return def(t, ...)
        end
    end
end

local metatypes = {"__call", "__le", "__lt", "__eq", "__len", "__concat", "__unm", "__pow", "__mod", "__div", "__mul", "__sub", "__add"}
local mt = {
    __index = metafun("__index", function(t,k)
        local pc = per_class[ffi.string(t.type.name)]
        if pc and pc[k] then return pc[k] end
        return base.get(t, k) or base[k] 
    end),
    __newindex = metafun("__newindex", function(t,k,v) base.set(t, k, v) end)
}
for _, v in pairs(metatypes) do
    mt[v] = metafun(v)
end
ffi.metatype("il_base", mt)

ffi.metatype("il_type", {
    __call = function(t, ...)
        local pc = per_class[ffi.string(t.name)]
        if pc and pc.__call then
            return pc.__call(t, ...)
        end
        return t.create(t)
    end,
    __index = function(t,k)
        local pc = per_class[ffi.string(t.name)]
        if pc and pc[k] then return pc[k] end
        return t.storage:get(k) or base[k]
    end,
    __newindex = function(t,k,v)
        t.storage:set(k,v)
    end
})

ffi.metatype("struct il_storage", {
    __index = function(t,k) return base.get(t,k) or base[k] end,
    __newindex = base.set
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
--        struct = "some_ffi_structure" -- is either a name of a structure or a ctype of one
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
        T.create = function(_)
            local v = ffi.new(cons.struct or "il_base")
            v.type = T
            ffi.gc(v, ffi.C.il_unref)
            v.destructor = function(v)
                if cons.destructor then
                    cons.destructor(v)
                end
            end
            if cons.constructor then
                cons.constructor(v)
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

--- Returns a value from a storage type
-- @tparam storage v The storage object
-- @tparam string name The key to retrieve
-- @return Associated value, converted to a lua-compatible type
function base.get(v, name)
    --assert(ffi.istype("struct il_storage**", v), "Expected storage")
    --if v == nil then error "null storage" end
    assert(type(name) == "string", "Expected string")
    local tag = ffi.new("enum il_storagetype[1]");
    local size = ffi.new("size_t[1]")
    local data
    if ffi.istype("il_base", v) then
        data = ffi.C.il_base_get(v, name, size, tag)
    elseif ffi.istype("il_type", v) then
        data = ffi.C.il_type_get(v, name, size, tag)
    else
        data = ffi.C.il_storage_get(v, name, size, tag)
    end
    --local data = ffi.C.il_storage_get(v, name, ffi.cast("size_t*", size), ffi.cast("enum il_storagetype*", tag))
    --print("got "..tostring(data).."@"..tostring(size[0]).." "..tostring(tag[0]))
    tag = tag[0]
    size = size[0]
    if tag == "IL_STRING" then
        return ffi.string(data, size)
    elseif tag == "IL_INT" then
        assert(size == ffi.sizeof("int"))
        return tonumber(ffi.cast("int*", data)[0])
    elseif tag == "IL_FLOAT" then
        assert(size == ffi.sizeof("float"))
        return ffi.cast("float*", data)[0]
    elseif tag == "IL_STORAGE" then
        assert(size == ffi.sizeof("il_storage"))
        return ffi.cast("il_storage*", data)
    elseif tag == "IL_OBJECT" then
        assert(size >= ffi.sizeof("il_base"))
        return ffi.cast("il_base*", data)
    elseif tag == "IL_VOID" then
        if size == 0 then
            return nil
        end
        return data, size
    else
        error "Unknown tag in storage"
    end
end

--- Sets a value in a key storage
-- @tparam storage v The storage
-- @tparam string name The key
-- @param val The value to convert to a storage-compatible type
function base.set(v, name, val)
    --assert(ffi.istype("struct il_storage**", v), "Expected storage")
    --if v == nil then error "null storage" end
    assert(type(name) == "string", "Expected string")
    local t
    if ffi.istype("struct il_storage", val) then
        t = "IL_STORAGE"
    elseif ffi.istype("il_base", val) then
        t = "IL_OBJECT"
    else
        t = ({string="IL_STRING", number="IL_FLOAT"})[type(val)]
    end
    if not t then error("Unknown type") end
    local size = 0
    if t == "IL_STRING" then
        size = #val
    end
    local ptr
    if t == "IL_STRING" then
        ptr = ffi.cast("char*", val)
    elseif t == "IL_FLOAT" then
        ptr = ffi.new("float[1]", val)
    elseif t == "IL_STORAGE" or t == "IL_OBJECT" then
        ptr = val
    end
    if ffi.istype("il_base", v) then
        ffi.C.il_base_set(v, name, ptr, size, t)
    elseif ffi.istype("il_type", v) then
        ffi.C.il_type_set(v, name, ptr, size, t)
    else
        ffi.C.il_storage_set(v, name, ptr, size, t)
    end
end
    
setmetatable(base, {__call=function(self,...) return base.create(...) end})

return base

