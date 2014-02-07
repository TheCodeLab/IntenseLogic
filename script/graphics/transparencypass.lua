local ffi = require "ffi"
local base = require "common.base"
local stage = require "graphics.stage"

ffi.cdef [[

struct ilG_stage;

struct ilG_stage *ilG_transparencypass_new(struct ilG_context *context);
void ilG_transparencypass_track(struct ilG_stage *self, struct ilG_renderer *renderer);

]]

local tp = {}

function tp:track(r)
    assert(self ~= nil and r ~= nil)
    modules.graphics.ilG_transparencypass_track(self, r)
end

function tp.create(c)
    assert(c ~= nil)
    return modules.graphics.ilG_transparencypass_new(c)
end

setmetatable(tp, {__call = function(self, ...) return tp.create(...) end})

return tp

