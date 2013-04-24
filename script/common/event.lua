--- Event API
-- Provides a mechanism for binding and firing events in the engine.
-- @author tiffany

local ffi = require "ffi"

require "common.base"

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

ffi.metatype("ilE_registry", {
    __index = function(t,k) return event[k] end
})

event.registry = ffi.C.il_registry;

local packers = {}

--- Sets a function to convert its arguments into a void* and a size_t to pass to the engine
-- @tparam registry registry The registry to hook this packer for
-- @tparam string name The name of the event to pack
-- @tparam func fn A function which takes arbitrary args and returns a cdata<void*> and a size_t
function event.setPacker(registry, name, fn)
    packers[tostring(ffi.cast("void*", registry))..name] = fn
end

--- Fires off an event
-- @tparam registry registry The registry (and all that registry's parents) to send the event to
-- @tparam string name The event to fire
-- @param ... Data to pass with the event
function event.event(registry, name, ...)
    local data, size = packers[tostring(ffi.cast("void*", registry))..name](...)
    ffi.C.ilE_globalevent(registry, name, size, data);
end

--- Creates a new timer
-- The timer will fire off an event, with the same data, each interval
-- @tparam registry registry The registry to attach the timer to
-- @tparam string name The name of the event the timer will fire
-- @param interval Interval in seconds, as a {sec=number,usec=number}, or as a {sec,usec}
-- @param ... Data to pass with the event
function event.timer(registry, name, interval, ...)
    local tv = ffi.new("struct timeval")
    if type(interval) == "number" then -- floating point number in seconds
        tv.tv_sec = math.floor(interval)
        tv.tv_usec = interval - math.floor(interval)
    elseif type(interval) == "table" then -- table of second/usecond
        tv.tv_sec = interval[1] or interval.sec
        tv.tv_usec = interval[2] or interval.usec
    else
        error "Unknown time format for timer"
    end
    local data, size = packers[tostring(ffi.cast("void*", registry))..name](...)
    ffi.C.ilE_globalevent(registry, name, size, data, tv)
end

local unpackers = {}

--- Registers an unpacker
-- Converts from the engine's representation to the Lua representation
-- @tparam registry registry The registry to register this unpacker for
-- @tparam string name The name of the event to unpack
-- @tparam func fn The function to call
function event.setUnpacker(registry, name, fn)
    unpackers[tostring(ffi.cast("void*", registry)) .. name] = fn
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

--- Unpacks an event from native format
-- @tparam registry registry The registry the event came from
-- @tparam string name The name of the event
-- @tparam size_t size The size of the data
-- @tparam cdata<void*> data The value associated with the event
-- @return Returns the size and data if it was unsuccessful, otherwise returns a value based on the type of the event.
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
    --print(registry, name, size, data, ctx)
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

--- Registers an event to be called when an event fires
-- @tparam registry registry The registry to watch
-- @tparam string name The name of the event to watch for
-- @tparam func fn The function to call
function event.register(registry, name, fn)
    assert(ffi.istype("ilE_registry", registry), "Expected registry");
    assert(type(name) == "string", "Expected string");
    assert(type(fn) == "function", "Expected function");

    local key = tostring(ffi.cast("void*", registry));

    if not callbacks[key] then
        callbacks[key] = {}
    end
    if not callbacks[key][name] then
        callbacks[key][name] = {}
        ffi.C.ilE_register(registry, name, ffi.C.ILE_DONTCARE, ffi.C.ILE_ANY, lua_dispatch, nil);
    end

    callbacks[key][name][#callbacks[key][name] + 1] = fn;
end

return event;

