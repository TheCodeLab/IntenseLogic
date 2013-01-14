local ffi = require "ffi"

ffi.cdef [[

typedef float il_Vector3[3];

typedef float il_Vector4[4];

typedef float il_Quaternion[4];

typedef float il_Matrix[16];

il_Matrix il_Matrix_identity;

]]--[[il_Matrix il_Matrix_mul(il_Matrix a, il_Matrix b);

il_Vector4 il_Vector4_mul_m(il_Vector4 a, il_Matrix b);

il_Matrix il_Matrix_translate(il_Vector3 t);

il_Matrix il_Matrix_rotate_v(float a, il_Vector3 n);

il_Matrix il_Matrix_scale(il_Vector3 v);

il_Matrix il_Matrix_perspective(double fovy, double aspect, double znear, double zfar);

il_Matrix il_Matrix_rotate_q(il_Quaternion q);

int il_Matrix_invert(il_Matrix m, il_Matrix* invOut);

]]

local matrix = {}

local function wrap(ptr)
    local obj = {}
    obj.ptr = ptr
    setmetatable(obj, {__index=matrix});
    return obj;
end
matrix.wrap = wrap;

matrix.type = ffi.typeof "il_Matrix";

matrix.identity = wrap(ffi.C.il_Matrix_identity);

function matrix.create()
    local mat = matrix.type();
    ffi.copy(mat, matrix.identity.ptr, ffi.sizeof(matrix.type));
    return mat;
end

setmetatable(matrix, {__call=function(self,mat) return matrix.create(mat) end})

return matrix;

