local ffi = require "ffi"
local base = require "common.base"
local stage = require "graphics.stage"

ffi.cdef [[

struct ilG_stage *ilG_outpass(struct ilG_context *context);

extern il_type ilG_outpass_type;

]]

base.wrap "il.graphics.outpass" {
    struct = "ilG_stage";
    __call = function(self, ctx) return ffi.C.ilG_outpass(ctx) end
}

return ffi.C.ilG_outpass_type

