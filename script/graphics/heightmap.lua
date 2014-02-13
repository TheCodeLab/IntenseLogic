local ffi = require "ffi"

local base = require "common.base"

require 'graphics.tex'
require 'graphics.renderer'

ffi.cdef [[

ilG_renderer ilG_heightmap_new(unsigned w, unsigned h, ilG_tex height, ilG_tex normal, ilG_tex color);

]]

return modules.graphics.ilG_heightmap_new

