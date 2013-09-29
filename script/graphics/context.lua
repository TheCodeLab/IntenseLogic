local ffi       = require "ffi"
local drawable  = require "graphics.drawable"
local material  = require "graphics.material"
local texture   = require "graphics.texture"
local base      = require "common.base"

require "util.timeval"

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
    ILG_CONTEXT_DEBUG,
    ILG_CONTEXT_EXPERIMENTAL,
    ILG_CONTEXT_WIDTH,
    ILG_CONTEXT_HEIGHT
};

struct ilG_frame {
    struct timeval start, elapsed;
    struct {
        struct ilG_frame *next;
        struct ilG_frame *last;
    } ll;
};

typedef struct ilG_context {
    il_base base;
    /* Creation parameters */
    int complete;
    int contextMajor;
    int contextMinor;
    int forwardCompat;
    enum ilG_context_profile profile;
    int debugContext;
    int experimental;
    int startWidth;
    int startHeight;
    char *initialTitle;
    /* Context management */
    int valid;
    struct GLFWwindow *window;
    unsigned int fbtextures[ILG_CONTEXT_NUMATTACHMENTS], framebuffer;
    int width, height;
    struct {
        struct ilG_state **data;
        size_t length;
        size_t capacity;
    } stages; 
    struct {
        struct il_positionable **data;
        size_t length;
        size_t capacity;
    } positionables;
    struct {
        struct ilG_light **data;
        size_t length;
        size_t capacity;
    } lights;
    struct ilG_frame frames_head;
    struct timeval frames_sum, frames_average;
    size_t num_frames;
    char *title;
    /* Drawing */
    struct ilG_drawable3d* drawable;
    struct ilG_material* material;
    struct ilG_texture* texture;
    const struct ilG_bindable *drawableb, *materialb, *textureb;
    struct ilG_camera* camera;
    struct il_world* world;
    struct il_positionable *positionable;
    unsigned *texunits;
    size_t num_texunits;
    size_t num_active;
} ilG_context;

extern il_type ilG_context_type;

void ilG_context_hint(ilG_context *self, enum ilG_context_hint hint, int param);
void ilG_context_build(ilG_context *self);
void ilG_context_resize(ilG_context *self, int w, int h, const char *title);
void ilG_context_makeCurrent(ilG_context *self);
void ilG_context_setActive(ilG_context*);
void ilG_context_addStage(ilG_context* self, struct ilG_stage* stage, int num);
void ilG_context_clearStages(ilG_context *self);

]]

base.wrap "il.graphics.context" {
    struct = "ilG_context";
    build       = modules.graphics.ilG_context_build;
    resize      = modules.graphics.ilG_context_resize;
    setActive   = modules.graphics.ilG_context_setActive;
    addStage    = modules.graphics.ilG_context_addStage;
    clearStages = modules.graphics.ilG_context_clearStages;
    averageFrametime = function(self)
        return tonumber(self.frames_average.tv_sec) + tonumber(self.frames_average.tv_usec) / 1000000
    end;
    hint = function(self, name, val)
        if type(val) == 'string' then
            val = modules.graphics["ILG_CONTEXT_"..val:upper()]
        end
        modules.graphics.ilG_context_hint(self, modules.graphics["ILG_CONTEXT_"..string.upper(name)], val)
    end;
    __newindex = function(self, k, v)
        hint('major')
        hint('minor')
        hint('forward_compat')
        hint('profile')
        hint('debug')
        hint('experimental')
        hint('width')
        hint('height')
    end;
}

return modules.graphics.ilG_context_type;

