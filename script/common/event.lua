--- Event API
-- Provides a mechanism for binding and firing events in the engine.
-- @author tiffany

local ffi = require "ffi"

require "common.base"

ffi.cdef [[

//////////////////////////////////////////////////////////////////////////////
// event.h

typedef struct ilE_handler ilE_handler;

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

typedef void(*ilE_callback)(const ilE_handler* registry, size_t size, const void *data, void * ctx);

ilE_handler *ilE_handler_new();
ilE_handler *ilE_handler_new_with_name(const char *name);
ilE_handler *ilE_handler_timer(const struct timeval *tv);
ilE_handler *ilE_handler_watch(int fd, enum ilE_fdevent what);
void ilE_handler_destroy(ilE_handler *self);

void ilE_handler_name(ilE_handler *self, const char *name);

void ilE_handler_fire(ilE_handler *self, size_t size, const void *data);
void ilE_handler_fireasync(ilE_handler *self, size_t size, const void *data);

int ilE_register_real(ilE_handler* self, const char *name, enum ilE_behaviour behaviour, enum ilE_threading threads, ilE_callback callback, void * ctx);
void ilE_unregister(ilE_handler *self, int handle);
void ilE_dump(ilE_handler *self);

extern ilE_handler *ilE_shutdown;
extern ilE_handler *ilE_shutdownCallbacks;
extern ilE_handler *ilE_shutdownHandlers;

]]

local event = {}

ffi.metatype("ilE_handler", {
    __index = function(t,k) return event[k] end
})

event.shutdown = modules.common.ilE_shutdown
event.shutdownCallbacks = modules.common.ilE_shutdownCallbacks
event.ilE_shutdownHandlers = modules.common.ilE_shutdownHandlers

local packers = {}

--- Sets a function to convert its arguments into a void* and a size_t to pass to the engine
-- @tparam registry registry The registry to hook this packer for
-- @tparam string name The name of the event to pack
-- @tparam func fn A function which takes arbitrary args and returns a cdata<void*> and a size_t
function event.setPacker(handler, fn)
    local key = tostring(ffi.cast("void*", handler)) --string.format("%p", handler)
    packers[key] = fn
end

function event.nilPacker()
    return nil, 0
end

function event.defaultPackers(reg)
    event.setPacker(event.shutdown, event.nilPacker)
end

event.defaultPackers(event.registry)

--- Fires off an event
-- @tparam registry registry The registry (and all that registry's parents) to send the event to
-- @tparam string name The event to fire
-- @param ... Data to pass with the event
function event.fire(handler, ...)
    local key = tostring(ffi.cast("void*", handler)) --string.format("%p", handler)
    if not packers[key] then error("No packer for event "..ffi.string(handler.name)) end
    local data, size = packers[key](...)
    modules.common.ilE_handler_fire(handler, size, data)
end

function event.fireAsync(handler, ...)
    local key = tostring(ffi.cast("void*", handler)) --string.format("%p", handler)
    if not packers[key] then error("No packer for event "..ffi.string(handler.name)) end
    local data, size = packers[key](...)
    modules.common.ilE_handler_fireasync(handler, size, data)
end

--[[function event.timer(registry, name, interval, ...)
        error "Unknown time format for timer"
    end
    local packer = packers[tostring(ffi.cast("void*", registry))..name]
    if not packer then error("No packer for event "..name) end
    local data, size = packer(...)
    modules.common.ilE_globaltimer(registry, name, size, data, tv)
end]]

local unpackers = {}

--- Registers an unpacker
-- Converts from the engine's representation to the Lua representation
-- @tparam registry registry The registry to register this unpacker for
-- @tparam string name The name of the event to unpack
-- @tparam func fn The function to call
function event.setUnpacker(handler, fn)
    local key = tostring(ffi.cast("void*", handler)) --string.format("%p", handler)
    unpackers[key] = fn
end

function event.nilUnpacker()
    return
end

function event.arrayUnpacker(T, n)
    return function(size, data)
        local t = {}
        local arr = ffi.cast(T.."*", data)
        for i = 0, n do
            t[#t + 1] = arr[i]
        end
        return unpack(t)
    end
end

function event.typeUnpacker(T)
    return event.arrayUnpacker(T, 1)
end

function event.defaultUnpackers(reg)
    event.setUnpacker(event.shutdown, event.nilUnpacker)
    event.setUnpacker(event.shutdownCallbacks, event.nilUnpacker)
    event.setUnpacker(event.shutdownHandlers, event.nilUnpacker)
end

event.defaultUnpackers(event.registry)

--- Unpacks an event from native format
-- @tparam registry registry The registry the event came from
-- @tparam string name The name of the event
-- @tparam size_t size The size of the data
-- @tparam cdata<void*> data The value associated with the event
-- @return Returns the size and data if it was unsuccessful, otherwise returns a value based on the type of the event.
function event.unpack(handler, size, data)
    local key = tostring(ffi.cast("void*", handler)) --string.format("%p", handler)
    assert(unpackers[key], "No unpacker")
    return unpackers[key](size, data)
end

local callbacks = {}

function lua_dispatch(handler, size, data, ctx)
    local key = tostring(ffi.cast("void*", handler)) --string.format("%p", handler);
    local args = {event.unpack(handler, size, data)}
    if not callbacks[key] then return end
    for i = 1, #callbacks[key] do
        local f = function()
            callbacks[key][i](handler, unpack(args))
        end
        local res, err = xpcall(f, function(s) print(debug.traceback(s, 2)) end);
        -- TODO: get proper error propogation when we 'escape' protected calling by being a callback from C
        if not res then
            table.remove(callbacks[key], i)
        end
    end
end

--- Registers an event to be called when an event fires
-- @tparam registry registry The registry to watch
-- @tparam string name The name of the event to watch for
-- @tparam func fn The function to call
function event.register(handler, fn)
    assert(ffi.istype("ilE_handler", handler), "Expected handler, got "..type(handler))
    assert(handler ~= nil, "Expected handler, got NULL")
    assert(type(fn) == "function", "Expected function, got "..type(fn))

    local key = tostring(ffi.cast("void*", handler)) --string.format("%p", handler);
    local info = debug.getinfo(fn, "n")
    local name = info.name or "<anonymous>"

    if not callbacks[key] then
        callbacks[key] = {}
        modules.common.ilE_register_real(handler, name, modules.common.ILE_DONTCARE, modules.common.ILE_ANY, lua_dispatch, nil);
    end

    callbacks[key][#callbacks[key] + 1] = fn;
end

function event.create(arg, name)
    local h
    if not arg then -- normal
        h = modules.common.ilE_handler_new(arg)
    elseif type(arg) == "string" then -- normal with name
        h = modules.common.ilE_handler_new_with_name(arg, name)
    elseif type(arg) == "number" then -- timer in seconds
        local tv = ffi.new("struct timeval")
        tv.tv_sec = math.floor(arg)
        tv.tv_usec = (arg - math.floor(arg)) * 1000000
        h = modules.common.ilE_handler_timer(tv)
    elseif type(arg) == "table" then -- timer of (second,usecond)
        local tv = ffi.new("struct timeval")
        tv.tv_sec = arg[1] or arg.sec
        tv.tv_usec = arg[2] or arg.usec
        h = modules.common.ilE_handler_timer(tv)
    elseif type(arg) == "userdata" then -- FILE*
        error("File watching NYI")
    else
        error("Expected nil, number, table, or FILE*")
    end
    if name then
        modules.common.ilE_handler_name(h, name)
    end
    h:setPacker(event.nilPacker)
    h:setUnpacker(event.nilUnpacker)
    return h
end

setmetatable(event, {__call=function(self,...)return event.create(...)end})

return event;

