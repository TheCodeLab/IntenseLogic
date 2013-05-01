local ffi = require "ffi"
local base = require "common.base"
local stage = require "graphics.stage"

ffi.cdef [[

struct ilG_stage;

void ilG_geometrypass(struct ilG_stage* self);

]]

return ffi.C.ilG_geometrypass -- TODO: something to make this less awkward

