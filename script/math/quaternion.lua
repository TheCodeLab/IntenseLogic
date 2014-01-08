--- Quaternion type
-- @type quaternion
local ffi = require "ffi"

require "math.scalar_defs"

local vector3-- = require "vector3"

ffi.cdef [[

il_quat il_quat_new();
void il_quat_free(il_quat q);
il_quat il_quat_copy(il_quat q);

il_quat il_quat_set(il_quat q, float x, float y, float z, float w);
il_quat il_quat_mul(const il_quat a, const il_quat b, il_quat q);
il_quat il_quat_lerp(const il_quat a, const il_quat b, float t, il_quat q);
il_quat il_quat_fromYPR(float yaw, float pitch, float roll, il_quat q);
il_quat il_quat_fromAxisAngle(float x, float y, float z, float a, il_quat q);
il_quat il_quat_normalize(const il_quat a, il_quat q);
float il_quat_len(const il_quat a);
float il_quat_dot(const il_quat a);

]]

local quaternion = {}

--- FFI CType of quaternions
quaternion.type = ffi.typeof("il_quat");

function index(t,k)
    vector3 = vector3 or require "math.vector3"
    if k == "x" then
        return t.ptr[0]
    elseif k == "y" then
        return t.ptr[1]
    elseif k == "z" then
        return t.ptr[2]
    elseif k == "w" or k == "i" then
        return t.ptr[3]
    elseif k == "v" then
        return vector3(t.x, t.y, t.z)
    elseif k == "len" then
        return il_quat_len(t.ptr)
    elseif k == "dot" then
        return il_quat_dot(t.ptr)
    elseif k == "normal" then
        return quaternion.wrap(il_quat_normalize(t.ptr, nil))
    end
    return quaternion[k]
end

function newindex(t, k, v)
    vector3 = vector3 or require "math.vector3"
    if k == "x" or k == "y" or k == "z" or k == "w" then
        assert(type(v) == "number")
        t.ptr[({x=0, y=1, z=2, w=3})[k]] = v
    elseif k == "v" then
        assert(vector3.check(v))
        t.ptr[0] = v.x
        t.ptr[1] = v.y
        t.ptr[2] = v.z
    elseif k == "i" then
        assert(type(v) == "number")
        t.ptr.w = v
    end
    error("Invalid key \""..tostring(k).."\" in quaternion")
end

function mul(a, b)
    vector3 = vector3 or require "math.vector3"
    if quaternion.check(b) then
        return quaternion.wrap(modules.math.il_quat_mul(a.ptr, b.ptr, nil))
    elseif vector3.check(b) then
        return b * a
    else
        error("Expected quaternion or vector3")
    end
end

function lerp(a, b, t)
    return quaternion.wrap(modules.math.il_quat_lerp(a,b,t,nil))
end

function gc(obj)
    modules.math.il_quat_free(obj.ptr)
end

function ts(self)
    return "["..self.ptr[0]..", "..self.ptr[1]..", "..self.ptr[2]..", "..self.ptr[3].."]"
end

--- Converts a cdata to a quaternion
function quaternion.wrap(ptr)
    local obj = {}
    obj.ptr = ptr;
    obj.T = "quat"
    setmetatable(obj, {__index = index, __newindex=newindex, __mul = mul, __gc = gc, __tostring = ts})
    return obj
end

--- Tests whether an object is a quaternion
function quaternion.check(obj)
    return type(obj) == "table" and obj.T == "quat" and ffi.istype(quaternion.type, obj.ptr)
end

--- Creates a new quaternion
-- Can be passed YPR, Axis (as vec3) Angle, and Axis (as numbers) Angle.
function quaternion.create(...)
    vector3 = vector3 or require "math.vector3"
    local args = {...};
    if #args == 1 then
        if ffi.istype("il_quat", args[1]) then
            return quaternion.wrap(args[1])
        elseif quaternion.check(args[1]) then
            return quaternion.wrap(modules.math.il_quat_copy(args[1].ptr))
        else
            assert(false)
        end
    elseif #args == 3 then -- YPR
        assert(type(args[1]) == "number" and type(args[2]) == "number" and type(args[3]) == "number")
        return quaternion.wrap(modules.math.il_quat_fromYPR(args[1], args[2], args[3], nil))
    elseif #args == 2 then -- axis angle with vec3
        assert(vector3.check(args[1]))
        assert(type(args[2]) == "number")
        return quaternion.wrap(modules.math.il_quat_fromAxisAngle(args[1].x, args[1].y, args[1].z, args[2], nil))
    elseif #args == 4 then -- raw quaternion
        assert(type(args[1]) == "number" and type(args[2]) == "number" and type(args[3]) == "number" and type(args[4]) == "number")
        --return quaternion.wrap(modules.math.il_quat_fromAxisAngle(args[1], args[2], args[3], args[4], nil))
        return quaternion.wrap(modules.math.il_quat_set(nil, args[1], args[2], args[3], args[4]))
    end
    error "Don't know how to handle args"
end

setmetatable(quaternion, {__call = function(self, ...) return quaternion.create(...) end})

return quaternion;

