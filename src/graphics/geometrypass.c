#include "geometrypass.h"

#include <sys/time.h>

#include "graphics/renderer.h"
#include "graphics/bindable.h"
#include "graphics/context.h"
#include "graphics/glutil.h"
#include "graphics/arrayattrib.h"
#include "graphics/drawable3d.h"
#include "util/ilassert.h"

struct ilG_geometry {
    IL_ARRAY(ilG_renderer,) renderers;
    il_table storage;
};

static void geometry_free(void *ptr)
{
    ilG_geometry *self = ptr;
    unsigned i;
    for (i = 0; i < self->renderers.length; i++) {
        ilG_renderer_free(self->renderers.data[i]);
    }
    IL_FREE(self->renderers);
    free(self);
}

static void geometry_draw(void *ptr)
{
    ilG_geometry *self = ptr;
    ilG_testError("Unknown");
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    ilG_testError("Could not setup to draw geometry");
    
    unsigned i;
    for (i = 0; i < self->renderers.length; i++) {
        ilG_renderer_draw(&self->renderers.data[i]);
        ilG_testError("Rendering %s", ilG_renderer_getName(&self->renderers.data[i]));
    }
}

static int geometry_build(void *ptr, ilG_context *ctx)
{
    ilG_geometry *self = ptr;
    for (unsigned i = 0; i < self->renderers.length; i++) {
        ilG_renderer_build(&self->renderers.data[i], ctx);
    }
    return 1;
}

static il_table *geometry_get_storage(void *ptr)
{
    ilG_geometry *self = ptr;
    return &self->storage;
}

static bool geometry_get_complete(const void *ptr)
{
    (void)ptr;
    return 1;
}

static void geometry_add_renderer(void *self, ilG_renderer renderer)
{
    IL_APPEND(((ilG_geometry*)self)->renderers, renderer);
}


const ilG_renderable ilG_geometry_renderer = {
    .free = geometry_free,
    .draw = geometry_draw,
    .build = geometry_build,
    .get_storage = geometry_get_storage,
    .get_complete = geometry_get_complete,
    .add_positionable = NULL,
    .add_renderer = geometry_add_renderer,
    .name = "Geometry Pass"
};

ilG_geometry *ilG_geometry_new(ilG_context *context)
{
    (void)context;
    ilG_geometry *self = calloc(1, sizeof(ilG_geometry));
    return self;
}


