local ffi = require "ffi"

require "scalar_defs"

local vector3-- = require "vector3"

ffi.cdef [[

il_Quaternion il_Quaternion_fromAxisAngle(il_Vector3 v, float a);
il_Quaternion il_Quaternion_fromYPR(float bank, float heading, float attitutde);
il_Quaternion il_Quaternion_mul(il_Quaternion a, il_Quaternion b);

]]

local quaternion = {}

quaternion.type = ffi.typeof("il_Quaternion");

function index(t,k)
    vector3 = vector3 or require "vector3"
    if k == "x" or k == "y" or k == "z" or k == "w" then
        return t.ptr[k]
    elseif k == "v" then
        return vector3(t.x, t.y, t.z)
    elseif k == "i" then
        return t.ptr.w
    end
    return quaternion[k]
end

function newindex(t, k, v)
    vector3 = vector3 or require "vector3"
    if k == "x" or k == "y" or k == "w" then
        assert(type(v) == "number")
        t.ptr[k] = v
    elseif k == "v" then
        assert(type(v) == "table" and ffi.istype(vector3.type, v.ptr))
        t.ptr.x = v.x
        t.ptr.y = v.y
        t.ptr.z = v.z
    elseif k == "i" then
        assert(type(v) == "number")
        t.ptr.w = v
    end
    error("Invalid key \""..tostring(k).."\" in quaternion")
end

function mul(a, b)
    assert(type(b) == "table" and ffi.istype(quaternion.type, b.ptr))
    return quaternion.wrap(ffi.C.il_Quaternion_mul(a.ptr, b.ptr))
end

function quaternion.wrap(ptr)
    local obj = {}
    obj.ptr = ptr;
    setmetatable(obj, {__index = index, __newindex=newindex, __mul = mul})
    return obj
end

function quaternion.create(...)
    vector3 = vector3 or require "vector3"
    args = {...};
    if type(args[1]) == "table" then -- axis angle
        assert(ffi.istype(vector3.type, args[1].ptr))
        assert(type(args[2]) == "number")
        return quaternion.wrap(ffi.C.il_Quaternion_fromAxisAngle(args[1].ptr, args[2]));
    elseif type(args[1]) == "number" then -- angles
        assert(type(args[1]) == "number" and type(args[2]) == "number" and type(args[3]) == "number");
        return quaternion.wrap(ffi.C.il_Quaternion_fromYPR(args[1], args[2], args[3]));
    end
    assert(false);
end

setmetatable(quaternion, {__call = function(self, ...) return quaternion.create(...) end})

return quaternion;

