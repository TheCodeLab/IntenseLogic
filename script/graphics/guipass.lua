local ffi = require "ffi"
local base = require "common.base"

require "graphics.context"
require "graphics.stage"

ffi.cdef [[

ilG_stage *ilG_guipass(ilG_context *context);

extern il_type ilG_guipass_type;

]]

base.wrap "il.graphics.guipass" {
    struct = "ilG_stage",
    __call = function(self, ...) return ffi.C.ilG_guipass(...) end
}

return ffi.C.ilG_guipass_type

