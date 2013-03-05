local ffi = require "ffi"

require "scalar_defs"

local vector4-- = require "vector3"

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

quaternion.type = ffi.typeof("il_quat");

function index(t,k)
    vector4 = vector4 or require "vector4"
    if k == "x" then
        return t.ptr[0]
    elseif k == "y" then
        return t.ptr[1]
    elseif k == "z" then
        return t.ptr[2]
    elseif k == "w" or k == "i" then
        return t.ptr[3]
    elseif k == "v" then
        return vector4(t.x, t.y, t.z)
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
    vector4 = vector4 or require "vector4"
    if k == "x" or k == "y" or k == "z" or k == "w" then
        assert(type(v) == "number")
        t.ptr[({x=0, y=1, z=2, w=3})[k]] = v
    elseif k == "v" then
        assert(vector4.check(v))
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
    assert(type(b) == "table" and ffi.istype(quaternion.type, b.ptr))
    return quaternion.wrap(ffi.C.il_quat_mul(a.ptr, b.ptr, nil))
end

function lerp(a, b, t)
    return quaternion.wrap(ffi.C.il_quat_lerp(a,b,t,nil))
end

function gc(obj)
    ffi.C.il_quat_free(obj.ptr)
end

function quaternion.wrap(ptr)
    local obj = {}
    obj.ptr = ptr;
    setmetatable(obj, {__index = index, __newindex=newindex, __mul = mul, __gc = gc})
    return obj
end

function quaternion.check(obj)
    return type(obj) == "table" and ffi.istype(quaternion.type, obj.ptr)
end

function quaternion.create(...)
    vector4 = vector4 or require "vector4"
    local args = {...};
    if #args == 3 then -- YPR
        assert(type(args[1]) == "number" and type(args[2]) == "number" and type(args[3]) == "number")
        return quaternion.wrap(ffi.C.il_quat_fromYPR(args[1], args[2], args[3], nil))
    elseif #args == 2 then -- axis angle with vec4
        assert(vector4.check(args[1]))
        assert(type(args[2]) == "number")
        return quaternion.wrap(ffi.C.il_quat_fromAxisAngle(args[1].x, args[1].y, args[1].z, args[2], nil))
    elseif #args == 4 then -- axis angle from numbers
        assert(type(args[1]) == "number" and type(args[2]) == "number" and type(args[3]) == "number" and type(args[4]) == "number")
        return quaternion.wrap(ffi.C.il_quat_fromAxisAngle(args[1], args[2], args[3], args[4], nil))
    end
    error "Don't know how to handle args"
end

setmetatable(quaternion, {__call = function(self, ...) return quaternion.create(...) end})

return quaternion;

