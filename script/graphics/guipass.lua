local ffi = require "ffi"
local base = require "common.base"

require "graphics.context"
require "graphics.stage"
require "graphics.gui.frame"

ffi.cdef [[

ilG_stage *ilG_guipass(ilG_context *context);
void ilG_guipass_setRoot(ilG_stage *self, ilG_gui_frame *root);

extern il_type ilG_guipass_type;

]]

base.wrap "il.graphics.guipass" {
    struct = "ilG_stage";
    setRoot = modules.graphics.ilG_guipass_setRoot;
    __call = function(self, ...) return modules.graphics.ilG_guipass(...) end;
}

return modules.graphics.ilG_guipass_type

