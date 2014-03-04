#include "transparencypass.h"

#include "graphics/renderer.h"
#include "graphics/context.h"
#include "graphics/bindable.h"
#include "graphics/drawable3d.h"
#include "graphics/arrayattrib.h"
#include "graphics/material.h"
#include "common/world.h"
#include "util/array.h"
#include "util/log.h"

struct ilG_transparency {
    IL_ARRAY(ilG_renderer,) renderers;
    il_table storage;
    ilG_context *context;
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
    self->context = context;
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

static void trans_add_renderer(ilG_transparency *self, ilG_renderer renderer)
{
    IL_APPEND(self->renderers, renderer);
}

static void trans_del_renderer(ilG_transparency *self, ilG_renderer node)
{
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
    il_error("No transparent object %s<%p>", node.vtable->name, node.obj);
}

static void trans_message(void *ptr, int type, il_value *v)
{
    switch (type) {
    case 1:
        trans_add_renderer(ptr, *(ilG_renderer*)il_value_tomvoid(v));
        break;
    case 2:
        trans_del_renderer(ptr, *(ilG_renderer*)il_value_tomvoid(v));
        break;
    }
}

static void trans_push_msg(void *ptr, int type, il_value v)
{
    ilG_transparency *self = ptr;
    if (self->context) {
        ilG_context_message(self->context, ilG_transparency_wrap(self), type, v);
    } else {
        trans_message(ptr, type, &v);
        il_value_free(v);
    }
}

const ilG_renderable ilG_transparency_renderer = {
    .free = trans_free,
    .draw = trans_draw,
    .build = trans_build,
    .get_storage = trans_get_storage,
    .get_complete = trans_get_complete,
    .add_renderer = 1,
    .del_renderer = 2,
    .message = trans_message,
    .push_msg = trans_push_msg,
    .name = "Transparency"
};

ilG_transparency *ilG_transparency_new(ilG_context *context)
{
    (void)context;
    struct ilG_transparency *self = calloc(1, sizeof(struct ilG_transparency));
    return self;
}


