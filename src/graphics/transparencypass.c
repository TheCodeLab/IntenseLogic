#include "transparencypass.h"

#include "graphics/stage.h"
#include "graphics/context.h"
#include "graphics/renderer.h"
#include "graphics/bindable.h"
#include "graphics/drawable3d.h"
#include "graphics/arrayattrib.h"
#include "graphics/material.h"
#include "graphics/texture.h"
#include "common/world.h"
#include "util/array.h"

struct ilG_transparencypass {
    ilG_stage stage;
    IL_ARRAY(ilG_renderer*,) renderers;
};

struct rinfo {
    il_positionable *positionable;
    ilG_drawable3d *drawable;
    ilG_material *material;
    ilG_texture *texture;
};

static void draw_transparency(ilG_stage *ptr)
{
    struct ilG_transparencypass *self = (struct ilG_transparencypass*)ptr;
    ilG_testError("Unknown");
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    ilG_testError("glEnable");

    unsigned i;
    for (i = 0; i < self->renderers.length; i++) { // TODO: Work out how to do transparency
        ilG_renderer_draw(self->renderers.data[i]);
        ilG_testError("Rendering %s", ilG_renderer_getName(self->renderers.data[i]));
    }
}

ilG_stage *ilG_transparencypass_new(ilG_context *context)
{
    struct ilG_transparencypass *self = calloc(1, sizeof(struct ilG_transparencypass));
    self->stage.context = context;
    self->stage.run = draw_transparency;
    self->stage.name = "Transparency Pass";
    return &self->stage;
}

void ilG_transparencypass_track(ilG_stage *self, ilG_renderer *renderer)
{
    IL_APPEND(((struct ilG_transparencypass*)self)->renderers, renderer);
}

