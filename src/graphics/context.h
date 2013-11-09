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
    IL_LIST(struct ilG_frame) ll;
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
    GLFWwindow *window;
    GLuint fbtextures[ILG_CONTEXT_NUMATTACHMENTS], framebuffer;
    int width, height;
    IL_ARRAY(struct ilG_stage*,) stages;
    IL_ARRAY(struct il_positionable*,) positionables; // tracker.c // TODO: move to geometry stage
    IL_ARRAY(struct ilG_light*,) lights; // TODO: move to lighting stage
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
int /*success*/ ilG_context_build(ilG_context *self);
int /*success*/ ilG_context_resize(ilG_context *self, int w, int h, const char *title);
void ilG_context_makeCurrent(ilG_context *self);
int /*success*/ ilG_context_setActive(ilG_context*);
void ilG_context_addStage(ilG_context* self, struct ilG_stage* stage, int num);
void ilG_context_clearStages(ilG_context *self);

#endif

