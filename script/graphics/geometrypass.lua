local ffi = require "ffi"
local base = require "common.base"
local stage = require "graphics.stage"

ffi.cdef [[

struct ilG_stage;

struct ilG_stage *ilG_geometrypass_new(struct ilG_context *context);
void ilG_geometrypass_track(struct ilG_stage *self, struct ilG_renderer *renderer);

]]

local gp = {}

function gp:track(r)
    assert(self ~= nil and r ~= nil)
    modules.graphics.ilG_geometrypass_track(self, r)
end

function gp.create(c)
    assert(c ~= nil)
    return modules.graphics.ilG_geometrypass_new(c)
end

setmetatable(gp, {__call = function(self, ...) return gp.create(...) end})
return gp

