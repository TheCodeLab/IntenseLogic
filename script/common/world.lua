local ffi = require "ffi";

require "base"

local context;

ffi.cdef [[

struct il_world* il_world_new();

typedef struct il_world {
    il_base base;
    struct {
        struct il_positionable* data;
        size_t length;
        size_t capacity;
    } objects;
    struct ilG_context* context;
} il_world;

void il_world_add(il_world*, struct il_positionable*);

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

local function ts(t)
    return "World"..t.ptr.id;
end

local function wrap(ptr)
    local obj = {}
    obj.ptr = ptr;
    setmetatable(obj, {__index=index, __newindex=newindex, __tostring=ts});
    return obj;
end
world.wrap = wrap

function world:add(pos)
    ffi.C.il_world_add(self.ptr, pos.ptr);
end

function world.create()
    return wrap(ffi.C.il_world_new());
end

setmetatable(world, {__call=world.create})

return world;

