local ffi = require "ffi"

require "scalar_defs"

ffi.cdef [[

typedef float *il_mat;

il_mat il_mat_new();
void il_mat_free(il_mat m);
il_mat il_mat_copy(il_mat m);

il_mat il_mat_set(il_mat m, il_vec4 a, il_vec4 b, il_vec4 c, il_vec4 d);
il_mat il_mat_mul(const il_mat a, const il_mat b, il_mat m);
il_mat il_mat_translate(const il_vec4 v, il_mat m);
il_mat il_mat_scale(const il_vec4 v, il_mat m);
il_mat il_mat_identity(il_mat m);
il_mat il_mat_perspective(il_mat m, float fovy, float aspect, float znear, float zfar);
il_mat il_mat_rotate(const il_quat q, il_mat m);
il_mat il_mat_invert(const il_mat a, il_mat m);

]]

local matrix = {}

local function mul(a,b)
    assert(type(a) == "table" and ffi.istype(matrix.type, a.ptr), "Expected matrix")
    assert(type(b) == "table" and ffi.istype(matrix.type, a.ptr), "Expected matrix")
    return wrap(ffi.C.il_mat_mul(a.ptr, b.ptr, nil));
end

local function unm(a)
    assert(type(a) == "table" and ffi.istype(matrix.type, a.ptr), "Expected matrix")
    return wrap(ffi.C.il_mat_invert(a, nil));
end

local function wrap(ptr)
    local obj = {}
    obj.ptr = ptr
    setmetatable(obj, {__index=matrix, __mul=mul, __unm=unm});
    return obj;
end
matrix.wrap = wrap;

function matrix.check(m)
    return type(m) == "table" and ffi.istype(matrix.type, m.ptr)
end

matrix.type = ffi.typeof "il_mat";

matrix.identity = wrap(ffi.C.il_mat_identity(nil));

function matrix.create()
    return wrap(ffi.C.il_mat_new());
end

function matrix.translate(v)
    assert(vector4.check(v))
    return wrap(ffi.C.il_mat_translate(v.ptr, nil));
end

function matrix.scale(v)
    assert(vector4.check(v))
    return wrap(ffi.C.il_mat_scale(v.ptr, nil))
end

function matrix.rotate(a, v)
    if type(a) == "table" then
        assert(quaternion.check(a))
        return wrap(ffi.C.il_mat_rotate(a.ptr, nil))
    --[[elseif type(a) == "number" then
        assert(type(v) == "table" and ffi.istype("il_Vector3", v.ptr), "Expected vector3")
        return wrap(ffi.C.il_Matrix_rotate_v(a, v.ptr));]]
    else
        error("Expected quaternion or vector4")
    end
end

function matrix.perspective(fovy, aspect, znear, zfar)
    return wrap(ffi.C.il_mat_perspective(nil, fovy, aspect, znear, zfar))
end

setmetatable(matrix, {__call=function(self,mat) return matrix.create(mat) end})

return matrix;

