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

struct ilG_frame {
    struct timeval start, elapsed;
    struct {
        struct ilG_frame *next;
        struct ilG_frame *last;
    } ll;
};

typedef struct ilG_context {
    il_base base;
    struct GLFWwindow *window;
    int complete;
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
    /*GLuint*/ unsigned int fbtextures[ILG_CONTEXT_NUMATTACHMENTS], framebuffer;
    int width, height;
    struct {
        struct ilG_stage **data;
        size_t size;
        size_t capacity;
    } stages;
    struct {
        struct il_positionable **data;
        size_t size;
        size_t capacity;
    } positionables;
    struct {
        struct ilG_light **data;
        size_t size;
        size_t capacity;
    } lights;
    struct ilG_frame frames_head;
    struct timeval frames_sum, frames_average;
    size_t num_frames;
} ilG_context;

extern il_type ilG_context_type;

void ilG_context_resize(ilG_context *self, int w, int h, const char *title);
void ilG_context_setActive(ilG_context*);
void ilG_context_addStage(ilG_context* self, struct ilG_stage* stage, int num);

]]

base.wrap "il.graphics.context" {
    struct = "ilG_context";
    resize = modules.graphics.ilG_context_resize;
    setActive = modules.graphics.ilG_context_setActive;
    addStage = modules.graphics.ilG_context_addStage;
    averageFrametime = function(self)
        return tonumber(self.frames_average.tv_sec) + tonumber(self.frames_average.tv_usec) / 1000000
    end
}

return modules.graphics.ilG_context_type;

