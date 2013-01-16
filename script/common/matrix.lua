local ffi = require "ffi"

require "scalar_defs"

ffi.cdef [[

il_Matrix il_Matrix_identity;

il_Matrix il_Matrix_mul(il_Matrix a, il_Matrix b);

il_Vector4 il_Vector4_mul_m(il_Vector4 a, il_Matrix b);

il_Matrix il_Matrix_translate(il_Vector3 t);

il_Matrix il_Matrix_rotate_v(float a, il_Vector3 n);

il_Matrix il_Matrix_scale(il_Vector3 v);

il_Matrix il_Matrix_perspective(double fovy, double aspect, double znear, double zfar);

il_Matrix il_Matrix_rotate_q(il_Quaternion q);

int il_Matrix_invert(il_Matrix m, il_Matrix* invOut);

]]

local matrix = {}

local function mul(a,b)
    assert(type(a) == "table" and ffi.istype(matrix.type, a.ptr), "Expected matrix")
    assert(type(b) == "table" and ffi.istype(matrix.type, a.ptr), "Expected matrix")
    return wrap(ffi.C.il_Matrix_mul(a.ptr, b.ptr));
end

local function unm(a)
    assert(type(a) == "table" and ffi.istype(matrix.type, a.ptr), "Expected matrix")
    local o = matrix.type();
    local res = ffi.C.il_Matrix_invert(a, o);
    if res == -1 then
        error "Failed to invert matrix"
    end
    return o;
end

local function wrap(ptr)
    local obj = {}
    obj.ptr = ptr
    setmetatable(obj, {__index=matrix, __mul=mul, __unm=unm});
    return obj;
end
matrix.wrap = wrap;

matrix.type = ffi.typeof "il_Matrix";

matrix.identity = wrap(ffi.C.il_Matrix_identity);

function matrix.create()
    local mat = matrix.type();
    ffi.copy(mat, matrix.identity.ptr, ffi.sizeof(matrix.type));
    return wrap(mat);
end

function matrix.translate(v)
    assert(type(v) == "table" and ffi.istype("il_Vector3", v.ptr), "Expected vector3")
    return wrap(ffi.C.il_Matrix_translate(v.ptr));
end

function matrix.scale(v)
    assert(type(v) == "table" and ffi.istype("il_Vector3", v.ptr), "Expected vector3")
    return wrap(ffi.C.il_Matrix_scale(v.ptr))
end

function matrix.rotate(a, v)
    if type(a) == "table" then
        assert(ffi.istype("il_Quaternion", a.ptr), "Expected quaternion")
        return wrap(ffi.C.il_Matrix_rotate_q(a.ptr))
    elseif type(a) == "number" then
        assert(type(v) == "table" and ffi.istype("il_Vector3", v.ptr), "Expected vector3")
        return wrap(ffi.C.il_Matrix_rotate_v(a, v.ptr));
    else
        error("Expected quaternion or vector3")
    end
end

function matrix.perspective(fovy, aspect, znear, zfar)
    return wrap(ffi.C.il_Matrix_perspective(fovy, aspect, znear, zfar))
end

setmetatable(matrix, {__call=function(self,mat) return matrix.create(mat) end})

return matrix;

