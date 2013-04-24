local ffi = require "ffi"

require "math.scalar_defs"

local positionable = require "common.positionable"
local vector3 = require "math.vector3"

ffi.cdef [[

typedef unsigned int GLuint; // TODO: fix this somehow

struct il_positionable;

enum ilG_light_type {
    ILG_POINT,
    ILG_DIRECTIONAL
};

typedef struct ilG_light {
    struct il_positionable* positionable;
    il_vec3 color;
    enum ilG_light_type type;
    GLuint texture; // shadow map
    float radius;
} ilG_light;

ilG_light* ilG_light_new();
void ilG_light_add(ilG_light*, struct ilG_context* context);

]]

local light = {}

local function index(t, k)
    if k == "positionable" then
        return positionable.wrap(t.ptr.positionable);
    elseif k == "color" then
        return vector3.wrap(t.ptr.color)
    elseif k == "type" then
        return t.ptr['type']
    elseif k == "radius" then
        return t.ptr.radius
    end
    return light[k]
end

local function newindex(t, k, v)
    if k == "positionable" then
        assert(type(v) == "table" and ffi.istype("struct il_positionable*", v.ptr))
        t.ptr.positionable = v.ptr
    elseif k == "color" then
        assert(vector3.check(v))
        t.ptr.color = v.ptr
    elseif k == "type" then
        assert(type(v) == "number")
        t.ptr['type'] = v
    elseif k == "radius" then
        assert(type(v) == "number")
        t.ptr.radius = v
    end
end

function light.wrap(ptr)
    local obj = {ptr=ptr}
    setmetatable(obj, {__index=index, __newindex=newindex})
    return obj
end

function light:add(ctx)
    assert(type(ctx) == "table" and ffi.istype("struct ilG_context*", ctx.ptr))
    ffi.C.ilG_light_add(self.ptr, ctx.ptr)
end

function light.create(...)
    local args = {...}
    local obj = light.wrap(ffi.C.ilG_light_new())
    local n
    if type(args[1]) == "table" then -- postionable or position
        if ffi.istype("struct il_positionable*", args[1].ptr) then
            obj.positionable = args[1]
        elseif vector3.check(args[1]) then
            obj.positionable = positionable(args[1])
        else
            error("Expected positionable or vector3")
        end
        n = 2
    elseif type(args[1]) == "number" then -- x y z coords
        obj.positionable = positionable()
        obj.positionable.position = vector3(args[1], args[2], args[3])
        n = 4
    end
    if type(args[n]) == "number" then -- radius
        obj.radius = args[n]
        n = n + 1
    end
    if type(args[n]) == "table" then -- color
        assert(ffi.istype(vector4.type, args[n].ptr))
        obj.color = args[n]
    elseif type(args[n]) == "number" then
        obj.color = vector3(args[n], args[n+1], args[n+2])
    end
    return obj
end

setmetatable(light, {__call = function(self, ...) return light.create(...) end})
return light

