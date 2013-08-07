local ffi = require "ffi"

require "graphics.stage"
require "graphics.texture"

ffi.cdef [[

void ilG_skyboxpass(ilG_stage *self, struct ilG_texture *skytex);

]]

return modules.graphics.ilG_skyboxpass

