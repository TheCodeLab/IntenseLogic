--- Provides the basis of the IL object system.
-- See examples/item.lua and examples/itemTest.lua for usage examples.
--
-- All types inheriting from base have access to the methods defined here. All objects are created the same way as well: either with `base:create` or with __call
-- @author tiffany

local ffi = require "ffi"

local event
local storage = require 'common.storage'

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
    il_table storage;
    il_base_init_fn constructor;
    il_base_free_fn destructor;
    il_base_copy_fn copy;
    const char *name;
    size_t size;
    il_type *parent;
};

struct il_base {
    int refs;
    il_base_free_fn free;
    il_table storage;
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
il_table *il_base_getStorage(void *obj);
il_table *il_type_getStorage(il_type *T);
size_t il_sizeof(const il_type* self);
il_type *il_typeof(void *obj);
il_base *il_new(il_type *type);
const char *il_name(il_type *type);
const void *il_cast(il_type* T, const char *to);
void il_impl(il_type* T, const char *name, void *impl);
char *strdup(const char*);

]]

local base = {}

local per_class = {}
local set_types = {}

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
        local tname = ffi.string(base.typeof(t).name)
        local pc = per_class[tname] or error("Unwrapped type '"..tname.."'!")
        --print(pc, k, tname)
        if pc[k] then return pc[k] end
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

--- Wraps an existing type
-- Similar to base.type, but instead of creating a new type and returning it, it sets an existing type's per-class data like base.type does for new types
function base.wrap(name)
    return function(cons)
        per_class[name] = cons
        if cons.struct and not set_types[cons.struct] then
            ffi.metatype(cons.struct, base.metatable)
            set_types[cons.struct] = true
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
        --ffi.gc(T, function() print "WTF: freed static type object" end)
        local cname = ffi.C.strdup(name) --ffi.new("char[?]", #name+1, name)
        cons.__cname = cname
        cons.__type = T
        --print(ffi.string(cname))
        --ffi.gc(cname, function() print "WTF: freed static type name" end)
        T.name = cname
        T.parent = cons.parent
        T.size = ffi.sizeof(cons.struct or "il_base")
        T.constructor = function(v)
            ffi.gc(v, function() print("freed "..v) end) --modules.common.il_unref)
            if cons.constructor then
                cons.constructor(v)
            end
        end
        T.destructor = cons.destructor
        T.copy = cons.copy
        return T
    end
end

--- Returns the type object that corresponds to the given object
-- @tparam base v The value to get the type of
-- @treturn type The type object
function base.typeof(v)
    --assert(ffi.istype("il_base*", v) or ffi.istype("il_base", v))
    return modules.common.il_typeof(v)
end

--- Creates a new instance of a type
-- @tparam type T The type object
-- @treturn base
function base.create(T)
    assert(ffi.istype("il_type*", T) or ffi.istype("il_type", T))
    return modules.common.il_new(T)
end

--- Copies an object
-- @tparam base v The object to copy
-- @treturn base The copied object
function base.copy(v)
    return modules.common.il_copy(v)
end

local function getStorage(p)
    local s
    if ffi.istype("il_type", p) then
        s = modules.common.il_type_getStorage(p)
    else
        s = modules.common.il_base_getStorage(p)
    end
    assert(s ~= nil)
    return s
end

--- Returns a value from a storage type
-- @tparam storage v The storage object
-- @tparam string name The key to retrieve
-- @return Associated value, converted to a lua-compatible type
function base.get(v, name)
    return getStorage(v)[name]
end

--- Sets a value in a key storage
-- @tparam storage v The storage
-- @tparam string name The key
-- @param val The value to convert to a storage-compatible type
function base.set(v, name, val)
    getStorage(v)[name] = val
end

setmetatable(base, {__call=function(self,...) return base.create(...) end})

return base

