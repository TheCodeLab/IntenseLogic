local ffi = require "ffi"

ffi.load("build/libilmath.so", true)

package.path = package.path.."script/?.lua;script/common/?.lua;script/graphics/?.lua"

return {
    matrix = require "matrix",
    vector3 = require "vector3",
    vector4 = require "vector4",
    quaternion = require "quaternion"
}

