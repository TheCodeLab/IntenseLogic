---
-- Common matrix operations for 3D graphics
-- @author tiffany
local ffi = require "ffi"

require "math.scalar_defs"

ffi.cdef [[

il_mat il_mat_zero();
il_mat il_mat_mul(const il_mat a, const il_mat b);
il_mat il_mat_translate(const il_vec4 v);
il_mat il_mat_scale(const il_vec4 v);
il_mat il_mat_identity();
il_mat il_mat_perspective(float fovy, float aspect, float znear, float zfar);
il_mat il_mat_rotate(const il_quat q);
il_mat il_mat_invert(const il_mat a);

]]

local matrix = {}

local function mul(a,b)
    assert(type(a) == "table" and ffi.istype(matrix.type, a.ptr), "Expected matrix")
    assert(type(b) == "table" and ffi.istype(matrix.type, a.ptr), "Expected matrix")
    return wrap(modules.math.il_mat_mul(a.ptr, b.ptr));
end

local function unm(a)
    assert(type(a) == "table" and ffi.istype(matrix.type, a.ptr), "Expected matrix")
    return wrap(modules.math.il_mat_invert(a));
end

--- Wraps a cdata with a metatable
-- @param ptr Pointer to cdata
-- @treturn tab Wrapper table
function matrix.wrap(ptr)
    local obj = {}
    obj.ptr = ptr
    setmetatable(obj, {__index=matrix, __mul=mul, __unm=unm});
    return obj;
end
local wrap = matrix.wrap;

--- Returns whether or not the parameter is a matrix
-- @treturn bool
function matrix.check(m)
    return type(m) == "table" and ffi.istype(matrix.type, m.ptr)
end

--- FFI ctype of matrix
matrix.type = ffi.typeof "il_mat";

--- Identity matrix
matrix.identity = wrap(modules.math.il_mat_identity());

--- Creates a new matrix
-- @treturn matrix Created matrix
function matrix.create()
    return wrap(modules.math.il_mat_zero());
end

--- Produces a translation using the provided vector
-- The matrix provided is:
--
--     1 0 0 x
--     0 1 0 y
--     0 0 1 z
--     0 0 0 w
--
-- @tparam vec4 v Translation vector
-- @treturn matrix Translation matrix
function matrix.translate(v)
    assert(vector4.check(v))
    return wrap(modules.math.il_mat_translate(v.ptr));
end

--- Produces a scaling matrix using the provided vector
-- The matrix provided is:
--
--     x 0 0 0
--     0 y 0 0
--     0 0 z 0
--     0 0 0 w
--
-- @tparam vec4 v Scaling vector
-- @treturn matrix Scaling matrix
function matrix.scale(v)
    assert(vector4.check(v))
    return wrap(modules.math.il_mat_scale(v.ptr))
end

--- Creates a rotation matrix
-- @tparam quaternion a The quaternion to create the matrix with
-- @treturn matrix A matrix that produces the rotation provided
function matrix.rotate(a, v)
    if type(a) == "table" then
        assert(quaternion.check(a))
        return wrap(modules.math.il_mat_rotate(a.ptr))
    --[[elseif type(a) == "number" then
        assert(type(v) == "table" and ffi.istype("il_Vector3", v.ptr), "Expected vector3")
        return wrap(modules.math.il_Matrix_rotate_v(a, v.ptr));]]
    else
        error("Expected quaternion or vector4")
    end
end

--- Creates a projection matrix
-- @tparam number fovy Field of view, top to bottom (not left to right, a fovy of 70 is actually on the large end)
-- @tparam number aspect Aspect ratio of the screen
-- @tparam number znear Clipping plane for objects close to the camera (set as far away as comfortable, to preserve Z buffer precision)
-- @tparam number zfar Clipping plane for objects far from the camera (set as close as comfortable, to preserve Z buffer precision)
-- @treturn matrix Projection matrix
function matrix.perspective(fovy, aspect, znear, zfar)
    return wrap(modules.math.il_mat_perspective(fovy, aspect, znear, zfar))
end

setmetatable(matrix, {__call=function(self,mat) return matrix.create(mat) end})

return matrix;

