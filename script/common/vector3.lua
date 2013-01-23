local ffi = require "ffi"
require "scalar_defs"

ffi.cdef [[

il_Vector3 il_Vector3_add(il_Vector3 a, il_Vector3 b);
il_Vector3 il_Vector3_add_f(il_Vector3 a, float b);
il_Vector3 il_Vector3_sub(il_Vector3 a, il_Vector3 b);
il_Vector3 il_Vector3_sub_f(il_Vector3 a, float b);
il_Vector3 il_Vector3_mul(il_Vector3 a, il_Vector3 b);
il_Vector3 il_Vector3_mul_f(il_Vector3 a, float b);
il_Vector3 il_Vector3_div(il_Vector3 a, il_Vector3 b);
il_Vector3 il_Vector3_div_f(il_Vector3 a, float b);
float il_Vector3_len(il_Vector3 v);
il_Vector3 il_Vector3_normalise(il_Vector3 v);
float il_Vector3_dot(il_Vector3 a, il_Vector3 b);
il_Vector3 il_Vector3_cross(il_Vector3 a, il_Vector3 b);
il_Vector3 il_Vector3_rotate_q(il_Vector3 p, il_Quaternion q);

]]

local vector3 = {}

vector3.type = ffi.typeof "il_Vector3"

local function c_wrap(c, c_f)
    return function(a, b)
        assert(type(a) == "table" and ffi.istype(vector3.type, a.ptr), "Expected vector3")
        if type(b) == "number" then
            return vector3.wrap(c_f(a.ptr, b))
        elseif type(b) == "table" and ffi.istype(vector3.type, b.ptr) then
            return vector3.wrap(c(a.ptr, b.ptr))
        else
            error("Expected vector3 or number");
        end
    end
end

local add = c_wrap(ffi.C.il_Vector3_add, ffi.C.il_Vector3_add_f)
local sub = c_wrap(ffi.C.il_Vector3_sub, ffi.C.il_Vector3_sub_f)
local mul = c_wrap(ffi.C.il_Vector3_mul, ffi.C.il_Vector3_mul_f)
local div = c_wrap(ffi.C.il_Vector3_div, ffi.C.il_Vector3_div_f)

local function index(t, k)
    if k == "len" then
        return ffi.C.il_Vector3_len(t.ptr)
    elseif k == "normal" then
        return ffi.C.il_Vector3_normalise(t.ptr)
    elseif k == "x" then
        return t.ptr.x;
    elseif k == "y" then
        return t.ptr.y;
    elseif k == "z" then
        return t.ptr.z;
    end
    return vector3[k]
end

local function newindex(t, k, v)
    assert(type(v) == "number")
    if k == "x" then
        t.ptr.x = v;
    elseif k == "y" then
        t.ptr.y = v;
    elseif k == "z" then
        t.ptr.z = v;
    end
    error("Invalid key "..tostring(k).." in vector3")
end

local function ts(t)
    return "["..t.x..", "..t.y..", "..t.z.."]";
end

local function wrap(ptr)
    local obj = {}
    obj.ptr = ptr;
    setmetatable(obj, {__index=index, __newindex=newindex, __tostring=ts, __add=add, __sub=sub, __mul=mul, __div=div})
    return obj;
end
vector3.wrap = wrap;

function vector3.create(x, y, z)
    if type(x) == "table" then
        x, y, z = unpack(x)
    end
    if x then
        assert( type(x) == "number" and
                type(y) == "number" and
                type(z) == "number",
                "Expected vector3 literal")
        return wrap(vector3.type(x, y, z));
    else
        return wrap(vector3.type());
    end
end

setmetatable(vector3, {__call=function(self, ...) return vector3.create(...) end})

return vector3;

