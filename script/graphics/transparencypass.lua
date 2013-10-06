local ffi = require "ffi"
local base = require "common.base"
local stage = require "graphics.stage"

ffi.cdef [[

struct ilG_stage;

void ilG_transparencypass(struct ilG_stage* self);

]]

return modules.graphics.ilG_transparencypass

