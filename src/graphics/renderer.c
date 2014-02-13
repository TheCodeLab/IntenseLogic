#include "renderer.h"

#include <stdbool.h>

#include "graphics/context.h"
#include "common/storage.h"
#include "graphics/material.h"
#include "graphics/drawable3d.h"
#include "util/array.h"
#include "util/log.h"

ilG_renderer ilG_renderer_wrap(void *obj, const ilG_renderable *vtable)
{
    return (ilG_renderer) {
        .obj = obj,
        .vtable = vtable
    };
}

void ilG_renderer_free(ilG_renderer self)
{
    self.vtable->free(self.obj);
}

struct build_ctx {
    ilG_renderer self;
    ilG_context *context;
};
static void build(void *ptr)
{
    struct build_ctx *self = ptr;
    self->self.vtable->build(self->self.obj, self->context);
    free(self);
}

void ilG_renderer_build(ilG_renderer *self, ilG_context *context)
{
    struct build_ctx *ctx = calloc(1, sizeof(struct build_ctx));
    ctx->self = *self;
    ctx->context = context;
    ilG_context_upload(context, build, ctx);
}

void ilG_renderer_draw(ilG_renderer *self)
{
    self->vtable->draw(self->obj);
}

bool ilG_renderer_isComplete(const ilG_renderer *self)
{
    return self->vtable->get_complete(self->obj);
}

const il_table *ilG_renderer_getStorage(const ilG_renderer *self)
{
    return self->vtable->get_storage(self->obj);
}

il_table *ilG_renderer_mgetStorage(ilG_renderer *self)
{
    return self->vtable->get_storage(self->obj);
}

const char *ilG_renderer_getName(const ilG_renderer *self)
{
    const char *name = self->vtable->name;
    if (name) {
        return self->vtable->name;
    }
    return "Unnamed";
}

int ilG_renderer_addPositionable(ilG_renderer *self, il_positionable pos)
{
    if (!self->vtable->add_positionable) {
        il_error("Attempt to add a positionable to renderer which does not handle them");
        return 0;
    }
    self->vtable->add_positionable(self->obj, pos);
    return 1;
}

int ilG_renderer_addRenderer(ilG_renderer *self, ilG_renderer r)
{
    if (!self->vtable->add_renderer) {
        il_error("Attempt to add a renderer to renderer which does not handle them");
        return 0;
    }
    self->vtable->add_renderer(self->obj, r);
    return 1;
}

