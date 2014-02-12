#include "transparencypass.h"

#include "graphics/renderer.h"
#include "graphics/context.h"
#include "graphics/bindable.h"
#include "graphics/drawable3d.h"
#include "graphics/arrayattrib.h"
#include "graphics/material.h"
#include "graphics/texture.h"
#include "common/world.h"
#include "util/array.h"

struct ilG_transparency {
    IL_ARRAY(ilG_renderer,) renderers;
    il_table storage;
};

static void trans_free(void *ptr)
{
    ilG_transparency *self = ptr;
    IL_FREE(self->renderers);
    il_table_free(self->storage);
    free(self);
}

static void trans_draw(void *ptr)
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
        ilG_renderer_draw(&self->renderers.data[i]);
        ilG_testError("Rendering %s", ilG_renderer_getName(&self->renderers.data[i]));
    }
}

static int trans_build(void *ptr, ilG_context *context)
{
    ilG_transparency *self = ptr;
    for (unsigned i = 0; i < self->renderers.length; i++) {
        ilG_renderer_build(&self->renderers.data[i], context);
    }
    return 1;
}

static il_table *trans_get_storage(void *ptr)
{
    ilG_transparency *self = ptr;
    return &self->storage;
}

static bool trans_get_complete(const void *ptr)
{
    (void)ptr;
    return true;
}

static void trans_add_renderer(void *ptr, ilG_renderer renderer)
{
    IL_APPEND(((ilG_transparency*)ptr)->renderers, renderer);
}

const ilG_renderable ilG_transparency_renderer = {
    .free = trans_free,
    .draw = trans_draw,
    .build = trans_build,
    .get_storage = trans_get_storage,
    .get_complete = trans_get_complete,
    .add_positionable = NULL,
    .add_renderer = trans_add_renderer,
    .name = "Transparency"
};

ilG_transparency *ilG_transparency_new(ilG_context *context)
{
    (void)context;
    struct ilG_transparency *self = calloc(1, sizeof(struct ilG_transparency));
    return self;
}


