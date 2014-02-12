#include "renderer.h"

#include <stdbool.h>

#include "graphics/context.h"
#include "common/storage.h"
#include "graphics/material.h"
#include "graphics/drawable3d.h"
#include "graphics/texture.h"
#include "util/array.h"


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

int ilG_renderer_build(ilG_renderer *self, ilG_context *context)
{
    return self->vtable->build(self->obj, context);
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

