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

struct ilG_transparency {
    IL_ARRAY(ilG_renderer*,) renderers;
};

static void transparency_run(void *ptr)
{
    ilG_transparency *self = ptr;
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

static int transparency_track(void *ptr, ilG_renderer *renderer)
{
    IL_APPEND(((ilG_transparency*)ptr)->renderers, renderer);
    return 1;
}

const ilG_stagable ilG_transparency_stage = {
    .run = transparency_run,
    .track = transparency_track,
    .name = "Transparency Pass"
};

ilG_transparency *ilG_transparency_new(ilG_context *context)
{
    (void)context;
    struct ilG_transparency *self = calloc(1, sizeof(struct ilG_transparency));
    return self;
}


