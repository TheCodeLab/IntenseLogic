#include "renderer.h"

#include "graphics/context.h"
#include "graphics/context-internal.h"

static void handle_destroy(void *ptr)
{
    ilG_handle *self = ptr;
    ilG_renderman_delRenderer(&self->context->manager, self->id);
    free(self);
}

void ilG_handle_destroy(ilG_handle self)
{
    ilG_handle *ctx = calloc(1, sizeof(ilG_handle));
    *ctx = self;
    ilG_context_upload(self.context, handle_destroy, ctx);
}

bool ilG_handle_ready(ilG_handle self);
il_table *ilG_handle_storage(ilG_handle self);
const char *ilG_handle_getName(ilG_handle self);
const char *ilG_handle_getError(ilG_handle self);

void ilG_handle_addCoords(ilG_handle self, unsigned cosys, unsigned codata)
{
    ilG_context_msg msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = ILG_ADD_COORDS;
    msg.v.coords.parent = self.id;
    msg.v.coords.cosys = cosys;
    msg.v.coords.codata = codata;
    ilG_context_queue_produce(self.context->queue, msg);
}

void ilG_handle_delCoords(ilG_handle self, unsigned cosys, unsigned codata)
{
    ilG_context_msg msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = ILG_DEL_COORDS;
    msg.v.coords.parent = self.id;
    msg.v.coords.cosys = cosys;
    msg.v.coords.codata = codata;
    ilG_context_queue_produce(self.context->queue, msg);
}

void ilG_handle_setViewCoords(ilG_handle self, ilG_cosysid cosys)
{
    ilG_context_msg msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = ILG_VIEW_COORDS;
    msg.v.coords.parent = self.id;
    msg.v.coords.cosys = cosys;
    ilG_context_queue_produce(self.context->queue, msg);
}

void ilG_handle_addRenderer(ilG_handle self, ilG_handle node)
{
    ilG_context_msg msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = ILG_ADD_RENDERER;
    msg.v.renderer.parent = self.id;
    msg.v.renderer.child = node.id;
    ilG_context_queue_produce(self.context->queue, msg);
}

void ilG_handle_delRenderer(ilG_handle self, ilG_handle node)
{
    ilG_context_msg msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = ILG_DEL_RENDERER;
    msg.v.renderer.parent = self.id;
    msg.v.renderer.child = node.id;
    ilG_context_queue_produce(self.context->queue, msg);
}

void ilG_handle_addLight(ilG_handle self, struct ilG_light light)
{
    ilG_context_msg msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = ILG_ADD_LIGHT;
    msg.v.light.parent = self.id;
    msg.v.light.child = light;
    ilG_context_queue_produce(self.context->queue, msg);
}

void ilG_handle_delLight(ilG_handle self, struct ilG_light light)
{
    ilG_context_msg msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = ILG_DEL_LIGHT;
    msg.v.light.parent = self.id;
    msg.v.light.child = light;
    ilG_context_queue_produce(self.context->queue, msg);
}

void ilG_handle_message(ilG_handle self, int type, il_value data)
{
    ilG_context_msg msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = ILG_MESSAGE;
    msg.v.message.node = self.id;
    msg.v.message.type = type;
    msg.v.message.data = data;
    ilG_context_queue_produce(self.context->queue, msg);
}
