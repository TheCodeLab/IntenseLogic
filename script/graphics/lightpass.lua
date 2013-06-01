local ffi = require "ffi"

local base = require "common.base"
local stage = require "graphics.stage"
local context = require "graphics.context"

ffi.cdef [[

ilG_stage *ilG_lightpass(ilG_context* context);

extern il_type ilG_lightpass_type;

]]

base.wrap "il.graphics.lightpass" {
    struct = "ilG_stage";
    __call = function(self, ...) return modules.graphics.ilG_lightpass(...) end;
}

return modules.graphics.ilG_lightpass_type

