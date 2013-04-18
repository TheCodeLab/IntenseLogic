local ffi = require "ffi"

require "base"

ffi.cdef [[

//////////////////////////////////////////////////////////////////////////////
// event.h

struct il_type;
struct il_base;

typedef struct ilE_registry ilE_registry;

enum ilE_behaviour {
    ILE_DONTCARE,
    ILE_BEFORE,
    ILE_AFTER,
    ILE_OVERRIDE
};

enum ilE_threading {
    ILE_ANY,
    ILE_MAIN,
    ILE_TLS,
};

typedef void(*ilE_callback)(const ilE_registry* registry, const char *name, size_t size, const void *data, void * ctx);

ilE_registry* ilE_registry_new();

void ilE_registry_forward(ilE_registry *from, ilE_registry *to);

void ilE_globalevent(   ilE_registry* registry, const char *name, size_t size, const void *data);
void ilE_typeevent  (   struct il_type* type,   const char *name, size_t size, const void *data);
void ilE_objectevent(   struct il_base* base,   const char *name, size_t size, const void *data);

void ilE_globaltimer(   ilE_registry* registry, const char *name, size_t size, const void *data, struct timeval tv);
void ilE_typetimer  (   struct il_type* type,   const char *name, size_t size, const void *data, struct timeval tv);
void ilE_objecttimer(   struct il_base* base,   const char *name, size_t size, const void *data, struct timeval tv);

int ilE_register(ilE_registry* registry, const char *name, enum ilE_behaviour behaviour, enum ilE_threading threads, ilE_callback callback, void * ctx);

void ilE_dumpHooks(ilE_registry *registry);

extern ilE_registry* il_registry;

//////////////////////////////////////////////////////////////////////////////
// input.h

typedef struct ilI_mouseMove {
  int x, y;
} ilI_mouseMove;

typedef struct ilI_mouseWheel {
  int x, y;
} ilI_mouseWheel;

]]

local event = {}

function event.wrap_registry(ptr)
    local obj = {}
    obj.ptr = ptr;
    setmetatable(obj, {__index = event})
    return obj;
end
event.registry = event.wrap_registry(ffi.C.il_registry);

local unpackers = {}

function event.setUnpacker(registry, name, fn)
    unpackers[tostring(ffi.cast("void*", registry.ptr)) .. name] = fn
end

local function nilUnpacker()
    return nil
end
event.setUnpacker(event.registry, "startup", nilUnpacker)
event.setUnpacker(event.registry, "tick", nilUnpacker)
event.setUnpacker(event.registry, "shutdown", nilUnpacker)

local function intUnpacker(size, data)
    return ffi.cast("int", data)
end
event.setUnpacker(event.registry, "input.keydown", intUnpacker)
event.setUnpacker(event.registry, "input.keyup", intUnpacker)
event.setUnpacker(event.registry, "input.mousedown", intUnpacker)
event.setUnpacker(event.registry, "input.mouseup", intUnpacker)

local function mousemoveUnpacker(size, data)
    local s = ffi.cast("ilI_mouseMove*", data);
    return s.x, s.y
end
event.setUnpacker(event.registry, "input.mousemove", mousemoveUnpacker)

local function mousewheelUnpacker(size, data)
    local s = ffi.cast("ilI_mouseWheel*", data);
    return s.y, s.x;
end
event.setUnpacker(event.registry, "input.mousehweel", mousewheelUnpacker)

function event.unpack(registry, name, size, data)
    local key = tostring(ffi.cast("void*", registry)) .. name
    if unpackers[key] then
        return unpackers[key](size, data);
    end
    return size, data;
end

local callbacks = {}

function lua_dispatch(registry, name, size, data, ctx)
    local key = tostring(ffi.cast("void*", registry));
    name = ffi.string(name)
    print(registry, name, size, data, ctx)
    if not callbacks[key] or not callbacks[key][name] then return end
    for i = 1, #callbacks[key][name] do
        local res, err = pcall(callbacks[key][name][i], registry, name, event.unpack(registry, name, size, data));
        -- TODO: get proper error propogation when we 'escape' protected calling by being a callback from C
        if not res then
            print(err)
            table.remove(callbacks[key][name], i)
        end
    end
end

function event.register(registry, name, fn)
    assert(type(registry) == "table" and ffi.istype("ilE_registry*", registry.ptr), "Expected registry");
    assert(type(name) == "string", "Expected string");
    assert(type(fn) == "function", "Expected function");

    local key = tostring(ffi.cast("void*", registry.ptr));

    if not callbacks[key] then
        callbacks[key] = {}
    end
    if not callbacks[key][name] then
        callbacks[key][name] = {}
        ffi.C.ilE_register(registry.ptr, name, ffi.C.ILE_DONTCARE, ffi.C.ILE_ANY, lua_dispatch, nil);
    end

    callbacks[key][name][#callbacks[key][name] + 1] = fn;
end

return event;

