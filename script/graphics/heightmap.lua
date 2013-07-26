local ffi = require "ffi"

local base = require "common.base"

ffi.cdef [[

struct ilG_drawable3d *ilG_heightmap_new(struct ilG_context *context, unsigned w, unsigned h);
struct ilG_material *ilG_heightmap_shader(struct ilG_context *context);

]]

local hm = {}

hm.heightmap = modules.graphics.ilG_heightmap_new

function hm.defaultShader(...)
    local res = modules.graphics.ilG_heightmap_shader(...)
    if (res == nil) then
        error "Failed to compile shader"
    end
    return res
end

setmetatable(hm, {__call = function(_, ...) return hm.heightmap(...) end})

return hm

