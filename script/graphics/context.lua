local ffi       = require "ffi"
local drawable  = require "graphics.drawable"
local material  = require "graphics.material"
local texture   = require "graphics.texture"
local base      = require "common.base"

require "util.timeval"
require "common.event"
require "input.input"

ffi.cdef[[

enum ilG_context_attachments {
    ILG_CONTEXT_DEPTH,
    ILG_CONTEXT_ACCUM,
    ILG_CONTEXT_NORMAL,
    ILG_CONTEXT_DIFFUSE,
    ILG_CONTEXT_SPECULAR,
    ILG_CONTEXT_NUMATTACHMENTS
};

enum ilG_context_profile {
    ILG_CONTEXT_NONE,
    ILG_CONTEXT_CORE,
    ILG_CONTEXT_COMPAT
};

enum ilG_context_hint {
    ILG_CONTEXT_MAJOR,
    ILG_CONTEXT_MINOR,
    ILG_CONTEXT_FORWARD_COMPAT,
    ILG_CONTEXT_PROFILE,
    ILG_CONTEXT_DEBUG_CONTEXT,
    ILG_CONTEXT_EXPERIMENTAL,
    ILG_CONTEXT_WIDTH,
    ILG_CONTEXT_HEIGHT,
    ILG_CONTEXT_HDR,
    ILG_CONTEXT_USE_DEFAULT_FB,
    ILG_CONTEXT_DEBUG_RENDER,
};

struct ilG_frame {
    struct timeval start, elapsed;
    struct {
        struct ilG_frame *next;
        struct ilG_frame *last;
    } ll;
};

typedef struct ilG_context {
    il_table storage;
    int width, height;
    struct ilG_frame frames_head;
    struct timeval frames_sum,
                   frames_average;
    size_t num_frames;
    char *title;
    struct ilG_camera* camera;
    struct il_world* world;
    ilE_handler *tick,
                *on_resize,
                *close,
                *on_destroy;
    ilI_handler handler;
} ilG_context;

ilG_context *ilG_context_new();
void ilG_context_free(ilG_context *self);

void ilG_context_hint(ilG_context *self, enum ilG_context_hint hint, int param);
int ilG_context_build(ilG_context *self);
int ilG_context_resize(ilG_context *self, int w, int h, const char *title);
void ilG_context_makeCurrent(ilG_context *self);
int ilG_context_start(ilG_context*);
void ilG_context_addStage(ilG_context* self, struct ilG_stage* stage, int num);
void ilG_context_clearStages(ilG_context *self);

]]

local context = {}

-- TODO: Error propagation

function context:destroy()
    assert(self ~= nil)
    modules.graphics.ilG_context_free(self)
end

function context:build()
    assert(self ~= nil)
    local res = modules.graphics.ilG_context_build(self)
    assert(res ~= 0, "ilG_context_build")
end

function context:resize(w, h, title)
    assert(self ~= nil)
    local res = modules.graphics.ilG_context_resize(self, w, h, title)
    assert(res ~= 0, "ilG_context_resize")
end

function context:start()
    assert(self ~= nil)
    local res = modules.graphics.ilG_context_start(self)
    assert(res ~= 0, "ilG_context_start")
end

function context:addStage(stage, idx)
    if not idx then idx = -1 end
    assert(self ~= nil)
    assert(stage ~= nil)
    modules.graphics.ilG_context_addStage(self, stage, idx)
end

function context:clearStages()
    assert(self ~= nil)
    modules.graphics.ilG_context_clearStages(self)
end

function context:averageFrametime()
    assert(self ~= nil)
    return tonumber(self.frames_average.tv_sec) + tonumber(self.frames_average.tv_usec) / 1000000
end

function context:hint(name, val)
    assert(self ~= nil)
    assert(type(name) == 'string')
    if type(val) == 'string' then
        val = modules.graphics["ILG_CONTEXT_"..val:upper()]
    end
    modules.graphics.ilG_context_hint(self, modules.graphics["ILG_CONTEXT_"..string.upper(name)], val)
end

__newindex = function(self, k, v)
end

function context.create()
    return modules.graphics.ilG_context_new()
end

setmetatable(context, {
    __call = function(self, ...) return context.create(...) end
})

ffi.metatype("ilG_context", {
    __index = function(self, k)
        return context[k] or self.storage[k]
    end,
    __newindex = function(self, k, v)
        local h = function(n)
            if k == n then
                self:hint(k, v)
                return true
            end
            return false
        end
        local _ = h('major')
        or h('minor')
        or h('forward_compat')
        or h('profile')
        or h('debug')
        or h('experimental')
        or h('width')
        or h('height')
        or (function() self.storage[k] = v; return true end)
    end
})

return context

