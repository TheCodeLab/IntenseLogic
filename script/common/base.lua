---
-- Provides the basis of the IL object system.
-- See examples/item.lua and examples/itemTest.lua for usage examples.
-- @author tiffany

local ffi = require "ffi"

local event

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
    IL_LUA,
};

struct il_storage;

typedef struct il_typeclass {
    const char *name; 
    UT_hash_handle hh;
} il_typeclass;

typedef struct il_base il_base;
typedef struct il_type il_type;

typedef void (*il_base_init_fn)(il_base*);
typedef il_base *(*il_base_copy_fn)(il_base*);
typedef void (*il_base_free_fn)(il_base*);

struct il_type {
    il_typeclass *typeclasses;
    struct il_storage *storage;
    il_base_init_fn constructor;
    const char *name;
    struct ilE_registry *registry;
    size_t size;
    il_type *parent;
};

struct il_base {
    int refs;
    struct il_storage *storage;
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
struct ilE_registry *il_base_registry(il_base *self);
struct ilE_registry *il_type_registry(il_type *self);
size_t il_sizeof(const il_type* self);
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
        local pc = per_class[ffi.string(base.typeof(t).name)]
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
base.metatable = {
    __index = metafun("__index", function(t,k)
        local pc = per_class[ffi.string(base.typeof(t).name)]
        if pc and pc[k] then return pc[k] end
        return base.get(t, k) or base.get(base.typeof(t), k) or base[k] 
    end),
    __newindex = metafun("__newindex", function(t,k,v) base.set(t, k, v) end)
}
for _, v in pairs(metatypes) do
    base.metatable[v] = metafun(v)
end
ffi.metatype("il_base", base.metatable)

ffi.metatype("il_type", {
    __call = function(t, ...)
        local pc = per_class[ffi.string(t.name)]
        if pc and pc.__call then
            return pc.__call(t, ...)
        end
        local v = t.create(t)
        if t.struct then
            return ffi.cast(t.struct.."*", v)
        end
        return v
    end,
    __index = function(t,k)
        local pc = per_class[ffi.string(t.name)]
        if pc and pc[k] then return pc[k] end
        return base.get(t, k) or base[k]
    end,
    __newindex = function(t,k,v)
        base.set(t,k,v)
    end
})

ffi.metatype("struct il_storage", {
    __index = function(t,k) return base.get(t,k) or base[k] end,
    __newindex = base.set
})

--- Wraps an existing type
-- Similar to base.type, but instead of creating a new type and returning it, it sets an existing type's per-class data like base.type does for new types
function base.wrap(name)
    return function(cons)
        per_class[name] = cons
        if cons.struct then
            ffi.metatype(cons.struct, base.metatable)
        end
    end
end

--- Creates a new type.
-- Has a bit of fun with syntax. Doesn't actually take 2 arguments, it returns a function which takes the second argument. Example: 
--
--    local my_type = base.type "my_type" {
--        parent = my_other_type,
--        constructor = function(self)
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
-- __call maybe be overridden if arguments to the type constructor are 
-- preferable (as there are no arguments to the constructor in the C code). 
-- See the examples for how to do this.
--
-- @tparam string name The name of the type
-- @tparam tab cons The construction table of the type
-- @treturn cdata The type object
function base.type(name)
    return function(cons)
        base.wrap(name)(cons)
        local T = ffi.new("il_type")
        T.name = name
        T.parent = cons.parent
        T.size = ffi.sizeof(cons.struct or "il_base")
        T.constructor = function(v)
            ffi.gc(v, ffi.C.il_unref)
            v.destructor = function(v)
                if cons.destructor then
                    cons.destructor(v)
                end
            end
            if cons.constructor then
                cons.constructor(v)
            end
        end
        return T
    end
end

--- Returns the type object that corresponds to the given object
-- @tparam base v The value to get the type of
-- @treturn type The type object
function base.typeof(v)
    --assert(ffi.istype("il_base*", v) or ffi.istype("il_base", v))
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
    if ffi.istype("il_type", v) then
        data = ffi.C.il_type_get(v, name, size, tag)
    else
        data = ffi.C.il_base_get(ffi.cast("il_base*",v), name, size, tag)--data = ffi.C.il_storage_get(v, name, size, tag)
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
    elseif tag == "IL_LUA" then
        assert(size == ffi.sizeof("int"))
        local int = ffi.cast("int*", data)
        return debug.getregistry()[tonumber(int[0])]
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
        if not t then t = "IL_LUA" end
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
        size = ffi.sizeof("float")
    elseif t == "IL_STORAGE" or t == "IL_OBJECT" then
        ptr = val
    elseif t == "IL_LUA" then
        table.insert(debug.getregistry(), val)
        ptr = ffi.new("int[1]", #debug.getregistry())
        size = ffi.sizeof("int")
    end
    if ffi.istype("il_type", v) then
        ffi.C.il_type_set(v, name, ptr, size, t)
    else
        ffi.C.il_base_set(ffi.cast("il_base*",v), name, ptr, size, t)
    end
end

--- Fires an event for the given base or type
-- @see event.event
function base.event(self, name, ...)
    event = event or require "common.event"
    if ffi.istype("il_base", self) then
        event.event(ffi.C.ilE_base_registry(self), name, ...)
    elseif ffi.istype("il_type", self) then
        event.event(ffi.C.ilE_type_registry(self), name, ...)
    end
end

--- Sets a timer for the given base or type
-- @see event.timer
function base.timer(self, name, ...)
    event = event or require "common.event"
    if ffi.istype("il_base", self) then
        event.timer(ffi.C.ilE_base_registry(self), name, ...)
    elseif ffi.istype("il_type", self) then
        event.timer(ffi.C.ilE_type_registry(self), name, ...)
    end

end

--- Registers a hook for the given base or type
-- @see event.register
function base.register(self, name, fn)
    event = event or require "common.event"
    if ffi.istype("il_base", self) then
        event.register(ffi.C.ilE_base_registry(self), name, fn)
    elseif ffi.istype("il_type", self) then
        event.register(ffi.C.ilE_type_registry(self), name, fn)
    end
end
    
setmetatable(base, {__call=function(self,...) return base.create(...) end})

return base

