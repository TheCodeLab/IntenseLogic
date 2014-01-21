--- Event API
-- Provides a mechanism for binding and firing events in the engine.
-- @author tiffany

local ffi = require "ffi"

require "common.base"
local storage = require 'common.storage'

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

typedef void(*ilE_callback)(const il_value *data, il_value *ctx);

ilE_handler *ilE_handler_new();
ilE_handler *ilE_handler_new_with_name(const char *name);
ilE_handler *ilE_handler_timer(const struct timeval *tv);
ilE_handler *ilE_handler_watch(int fd, enum ilE_fdevent what);
void ilE_handler_destroy(ilE_handler *self);

const char *ilE_handler_getName(const ilE_handler *self);

void ilE_handler_name(ilE_handler *self, const char *name);

void ilE_handler_fire(ilE_handler *self, const il_value *data);
void ilE_handler_fireasync(ilE_handler *self, il_value data);

int ilE_register_real(ilE_handler* self, const char *name, enum ilE_behaviour behaviour, enum ilE_threading threads, ilE_callback callback, il_value ctx);
void ilE_unregister(ilE_handler *self, int handle);
void ilE_dump(ilE_handler *self);

extern ilE_handler *ilE_shutdown;
extern ilE_handler *ilE_shutdownCallbacks;
extern ilE_handler *ilE_shutdownHandlers;

]]

local event = {}

ffi.metatype("ilE_handler", {
    __index = function(t,k) 
        if k == "name" then
            return ffi.string(modules.common.ilE_handler_getName(t))
        end
        return event[k]
    end
})

event.shutdown = modules.common.ilE_shutdown
event.shutdownCallbacks = modules.common.ilE_shutdownCallbacks
event.shutdownHandlers = modules.common.ilE_shutdownHandlers

--- Fires off an event
-- @tparam registry registry The registry (and all that registry's parents) to send the event to
-- @tparam string name The event to fire
-- @param ... Data to pass with the event
function event.fire(handler, ...)
    local t = {...}
    for i, v in pairs(t) do
        if not ffi.istype("il_value", v) then
            t[i] = storage.pack(v)
        end
    end
    if #t > 1 then
        t = storage.pack(t)
    else
        t = t[1]
    end
    modules.common.ilE_handler_fire(handler, t)
end

function event.fireAsync(handler, ...)
    local t = {...}
    for i, v in pairs(t) do
        if not ffi.istype("il_value", v) then
            t[i] = storage.pack(v)
        end
    end
    if #t > 1 then
        t = storage.pack(t)
    else
        t = t[1]
    end
    modules.common.ilE_handler_fireasync(handler, t)
end

local callbacks = {}

function lua_dispatch(data, ctx)
    ctx = storage.unpack(ctx)
    local key = tostring(ffi.cast("void*", ctx)) --string.format("%p", handler);
    if not callbacks[key] then return end
    for i = 1, #callbacks[key] do
        local f = function()
            callbacks[key][i](storage.flatten(data))
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
        modules.common.ilE_register_real(handler, name, modules.common.ILE_DONTCARE, modules.common.ILE_ANY, lua_dispatch, storage.pack(handler));
    end

    callbacks[key][#callbacks[key] + 1] = fn;
    return fn
end

function event.unregister(handler, id)
    local key = tostring(ffi.cast("void*", handler))
    for i, v in pairs(callbacks[key]) do
        if v == id then
            table.remove(callbacks[key], i)
            return
        end
    end
    error "No such id"
end

local lua_handlers = {}

event.register(event.shutdownHandlers, function()
    for _, v in pairs(lua_handlers) do
        modules.common.ilE_handler_destroy(v)
    end
end)

function event.destroy(hnd)
    for i, v in pairs(lua_handlers) do
        if v == hnd then
            table.remove(lua_handlers, i)
        end
    end
    modules.common.ilE_handler_destroy(hnd)
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
    lua_handlers[#lua_handlers+1] = h
    return h
end

setmetatable(event, {__call=function(self,...)return event.create(...)end})

return event;

