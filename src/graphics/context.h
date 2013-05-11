#ifndef ILG_CONTEXT_H
#define ILG_CONTEXT_H

#include <stdlib.h>
#include <GL/glew.h>

#include "util/array.h"
#include "graphics/light.h"
#include "common/base.h"

struct ilG_stage;

enum ilG_context_attachments {
    ILG_CONTEXT_DEPTH,
    ILG_CONTEXT_ACCUM,
    ILG_CONTEXT_NORMAL,
    ILG_CONTEXT_DIFFUSE,
    ILG_CONTEXT_SPECULAR,
    ILG_CONTEXT_NUMATTACHMENTS
};

typedef struct ilG_context {
    il_base base;
    int complete;
    struct ilG_drawable3d* drawable;
    struct ilG_material* material;
    struct ilG_texture* texture;
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
    IL_ARRAY(ilG_light*,) lights; // TODO: move to lighting stage
} ilG_context;

extern il_type ilG_context_type;

void ilG_context_resize(ilG_context *self, int w, int h);
void ilG_context_setActive(ilG_context*);
void ilG_context_addStage(ilG_context* self, struct ilG_stage* stage, int num);

#endif

