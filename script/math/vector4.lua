local ffi = require "ffi"
require "math.scalar_defs"

local quaternion;
local vector3;

ffi.cdef [[

il_vec4 il_vec4_new();
void il_vec4_free(il_vec4 vec);
il_vec4 il_vec4_copy(il_vec4 vec);

il_vec4 il_vec4_set(il_vec4 vec, float x, float y, float z, float w);
char *il_vec4_print(const il_vec4 v, char *buf, unsigned length);

il_vec4 il_vec4_add(const il_vec4 a, const il_vec4 b, il_vec4 vec);
il_vec4 il_vec4_sub(const il_vec4 a, const il_vec4 b, il_vec4 vec);
il_vec4 il_vec4_mul(const il_vec4 a, const il_vec4 b, il_vec4 vec);
il_vec4 il_vec4_div(const il_vec4 a, const il_vec4 b, il_vec4 vec);
float il_vec4_dot(const il_vec4 a, const il_vec4 b);
il_vec3 il_vec4_to_vec3(const il_vec4 a, il_vec4 vec);
float il_vec4_len(const il_vec4 a);

]]

local vector4 = {}

vector4.type = ffi.typeof "il_vec4"

local function c_wrap(c)
    return function(a, b)
        assert(vector4.check(a))
        if type(b) == "number" then
            b = vector4(b, b, b, b)
        end
        assert(vector4.check(b))
        return vector4.wrap(c(a.ptr, b.ptr, nil))
    end
end

local add = c_wrap(ffi.C.il_vec4_add)
local sub = c_wrap(ffi.C.il_vec4_sub)
local mul = c_wrap(ffi.C.il_vec4_mul)
local div = c_wrap(ffi.C.il_vec4_div)

local function index(t, k)
    vector3 = vector3 or require "math.vector3"
    if k == "len" or k == "length" then
        return ffi.C.il_vec4_len(t.ptr)
    elseif k == "vec3" then
        return vector3.wrap(ffi.C.il_vec4_to_vec3(t.ptr, nil))
    elseif k == "x" then
        return t.ptr[0];
    elseif k == "y" then
        return t.ptr[1];
    elseif k == "z" then
        return t.ptr[2];
    elseif k == "w" then
        return t.ptr[3];
    end
    return vector4[k]
end

local function newindex(t, k, v)
    assert(type(v) == "number")
    if k == "x" then
        t.ptr[0] = v
        return
    elseif k == "y" then
        t.ptr[1] = v
        return
    elseif k == "z" then
        t.ptr[2] = v
        return
    elseif k == "w" then
        t.ptr[3] = v
        return
    end
    error("Invalid key "..tostring(k).." in vector4")
end

local function ts(t)
    -- not really worth using il_vec4_print
    return "("..t.x..", "..t.y..", "..t.z..", "..t.w..")";
end

local function gc(obj)
    ffi.C.il_vec4_free(obj.ptr);
end

local function wrap(ptr)
    local obj = {}
    obj.ptr = ptr;
    obj.T = "vec4"
    setmetatable(obj, {__index=index, __newindex=newindex, __tostring=ts, __add=add, __sub=sub, __mul=mul, __div=div, __gc=gc})
    return obj;
end
vector4.wrap = wrap;

function vector4.check(obj)
    return type(obj) == "table" and obj.T == "vec4" and ffi.istype(vector4.type, obj.ptr)
end

function vector4.dot(a,b)
    assert(vector4.check(a) and vector4.check(b))
    return ffi.C.il_vec4_dot(a.ptr, b.ptr)
end

function vector4.create(x, y, z, w)
    if type(x) == "table" then
        x, y, z, w = unpack(x)
    end
    if x then
        assert( type(x) == "number" and
                type(y) == "number" and
                type(z) == "number",
                "Expected vector4 literal")
        return wrap(ffi.C.il_vec4_set(nil, x, y, z, type(w)=="number" and w or 1.0));
    else
        return wrap(ffi.C.il_vec4_new());
    end
end

setmetatable(vector4, {__call=function(self, ...) return vector4.create(...) end})

return vector4;

