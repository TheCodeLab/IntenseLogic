#ifndef ILG_CONTEXT_H
#define ILG_CONTEXT_H

#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <sys/time.h>

#include "util/array.h"
#include "util/list.h"
#include "common/base.h"
#include "graphics/bindable.h"

struct ilG_stage;

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
    IL_LIST(struct ilG_frame) ll;
};

typedef struct ilG_context {
    il_base base;
    GLFWwindow *window;
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
    GLuint fbtextures[ILG_CONTEXT_NUMATTACHMENTS], framebuffer;
    int width, height;
    IL_ARRAY(struct ilG_stage*,) stages;
    IL_ARRAY(struct il_positionable*,) positionables; // tracker.c // TODO: move to geometry stage
    IL_ARRAY(struct ilG_light*,) lights; // TODO: move to lighting stage
    struct ilG_frame frames_head;
    struct timeval frames_sum, frames_average;
    size_t num_frames;
    char *title;
} ilG_context;

extern il_type ilG_context_type;

void ilG_context_resize(ilG_context *self, int w, int h, const char *title);
void ilG_context_makeCurrent(ilG_context *self);
void ilG_context_setActive(ilG_context*);
void ilG_context_addStage(ilG_context* self, struct ilG_stage* stage, int num);

#endif

