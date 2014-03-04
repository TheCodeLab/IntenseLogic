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
    ilG_context *context;
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
    self->context = ctx;
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

static void geometry_del_renderer(void *ptr, ilG_renderer node)
{
    ilG_geometry *self = ptr;
    for (unsigned i = 0; i < self->renderers.length-1; i++) {
        if (self->renderers.data[i].obj == node.obj) {
            self->renderers.data[i] = self->renderers.data[--self->renderers.length];
            return;
        }
    }
    if (self->renderers.length > 0 && self->renderers.data[self->renderers.length-1].obj == node.obj) {
        --self->renderers.length;
        return;
    }
    il_error("Renderer %s<%p> does not own %s<%p>", ilG_geometry_renderer.name, ptr, node.vtable->name, node.obj);
}

static void geometry_message(void *ptr, int type, il_value *v)
{
    switch (type) {
    case 1:
        geometry_add_renderer(ptr, *(ilG_renderer*)il_value_tomvoid(v));
        break;
    case 2:
        geometry_del_renderer(ptr, *(ilG_renderer*)il_value_tomvoid(v));
        break;
    default:
        il_error("No such message %i for %s<%p>", type, ilG_geometry_renderer.name, ptr);
    }
}

static void geometry_push_msg(void *ptr, int type, il_value v)
{
    ilG_geometry *self = ptr;
    if (self->context) {
        ilG_context_message(self->context, ilG_geometry_wrap(self), type, v);
    } else {
        geometry_message(ptr, type, &v);
        il_value_free(v);
    }
}

const ilG_renderable ilG_geometry_renderer = {
    .free = geometry_free,
    .draw = geometry_draw,
    .build = geometry_build,
    .get_storage = geometry_get_storage,
    .get_complete = geometry_get_complete,
    .add_renderer = 1,
    .del_renderer = 2,
    .message = geometry_message,
    .push_msg = geometry_push_msg,
    .name = "Geometry Pass"
};

ilG_geometry *ilG_geometry_new(ilG_context *context)
{
    (void)context;
    ilG_geometry *self = calloc(1, sizeof(ilG_geometry));
    return self;
}


