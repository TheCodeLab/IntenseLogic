local ffi = require "ffi"

ffi.cdef [[

//////////////////////////////////////////////////////////////////////////////
// event.h

struct timeval;

typedef struct ilE_queue ilE_queue;

typedef struct ilE_event ilE_event;

enum ilE_behaviour {
    ILE_DONTCARE,
    ILE_BEFORE,
    ILE_AFTER,
    ILE_OVERRIDE
};

ilE_queue* il_queue;

typedef void(*ilE_callback)(const ilE_queue*, const ilE_event*, void * ctx);

ilE_queue* ilE_queue_new();

ilE_event* ilE_new(uint16_t eventid, uint8_t size, void * data);

uint16_t ilE_getID(const ilE_event* event);

void * ilE_getData(const ilE_event* event, size_t *size);

int ilE_push(ilE_queue* queue, ilE_event* event);

int ilE_timer(ilE_queue* queue, ilE_event* event, struct timeval * interval);

int ilE_register(ilE_queue* queue, uint16_t eventid, enum ilE_behaviour behaviour, ilE_callback callback, void * ctx);

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

event.type = ffi.typeof "ilE_event*";

local function index(t, k)
    if k == "id" then
        return ffi.C.ilE_getID(t.ptr);
    end
    return event[k];
end

local function newindex(t, k, v)
    error("Invalid key \""..tostring(k).."\" in event")
end

function event.wrap(ptr)
    local obj = {}
    obj.ptr = ptr;
    setmetatable(obj, {__index = index, __newindex=newindex});
    return obj;
end

function event.wrap_queue(ptr)
    local obj = {}
    obj.ptr = ptr;
    setmetatable(obj, {__index = event})
    return obj;
end
event.mainqueue = event.wrap_queue(ffi.C.il_queue);

function event.unpack(ev)
    assert(type(ev) == "table" and ffi.istype(event.type, ev.ptr), "Expected event");
    if getmetatable(ev).__unpack then
        return getmetatable(ev).__unpack(ev);
    end
    if ev.id == 0 or ev.id == 1 or ev.id == 2 or ev.id == 9 then -- startup, tick, shutdown, graphics tick
        return nil;
    elseif ev.id >= 3 and ev.id <= 6 then -- keydown, keyup, mousedown, mouseup
        return ffi.cast("int", ffi.C.ilE_getData(ev.ptr, nil));
    elseif ev.id == 7 then -- mousemove
        local s = ffi.cast("ilI_mouseMove", ffi.C.ilE_getData(ev.ptr, nil));
        return s.x, s.y
    elseif ev.id == 8 then -- mousewheel
        local s = ffi.cast("ilI_mouseWheel", ffi.C.ilE_getData(ev.ptr, nil));
        return s.y, s.x;
    end
    error "Unable to decompose event"
end

local callbacks = {}

function lua_dispatch(queue, ev, ctx)
    local id = ffi.C.ilE_getID(ev);
    local key = tostring(ffi.cast("void*", queue));
    for i = 1, #callbacks[key][id] do
        local res, err = pcall(callbacks[key][id][i], queue, event.wrap(ev));
        -- TODO: get proper error propogation when we 'escape' protected calling by being a callback from C
        if not res then
            print(err)
            table.remove(callbacks[key][id], i)
        end
    end
end

function event.register(queue, id, fn)
    assert(type(queue) == "table" and ffi.istype("ilE_queue*", queue.ptr), "Expected queue");
    assert(type(id) == "number", "Expected number");
    assert(type(fn) == "function", "Expected function");

    local key = tostring(ffi.cast("void*", queue.ptr));

    if not callbacks[key] then
        callbacks[key] = {}
    end
    if not callbacks[key][id] then
        callbacks[key][id] = {}
        ffi.C.ilE_register(queue.ptr, id, ffi.C.ILE_DONTCARE, lua_dispatch, nil);
    end

    callbacks[key][id][#callbacks[key][id] + 1] = fn;
end

return event;

