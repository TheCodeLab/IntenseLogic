local ffi = require "ffi";

local context;

ffi.cdef [[

struct il_world* il_world_new();

typedef struct il_world {
    unsigned int id;
    unsigned refs;
    size_t nobjects;
    struct il_positionable** objects;
    struct ilG_context* context;
} il_world;

]]

local world = {}

local function index(t, k)
    context = context or require "context"
    if k == "context" then
        return context.wrap(t.ptr.context);
    end
    return world[k]
end

local function newindex(t, k, v)
    assert(type(v) == "table");
    if k == "context" then
        assert(ffi.istype("struct ilG_context*", v.ptr), "Expected context")
        t.ptr.context = v.ptr;
        return;
    end
    error("Invalid key \""..tostring(k).."\" in context");
end

local function wrap(ptr)
    local obj = {}
    obj.ptr = ptr;
    setmetatable(obj, {__index=index, __newindex=newindex});
    return obj;
end
world.wrap = wrap

function world.create()
    return wrap(ffi.C.il_world_new());
end

setmetatable(world, {__call=world.create})

return world;

