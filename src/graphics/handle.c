#include "renderer.h"

#include "graphics/context.h"
#include "graphics/context-internal.h"

void ilG_handle_addCoords(ilG_handle self, unsigned cosys, unsigned codata)
{
    struct ilG_context_msg *msg = calloc(1, sizeof(struct ilG_context_msg));
    msg->type = ILG_ADD_COORDS;
    msg->value.coords.parent = self.id;
    msg->value.coords.cosys = cosys;
    msg->value.coords.codata = codata;
    ilG_context_queue_produce(self.context->queue, msg);
}

void ilG_handle_delCoords(ilG_handle self, unsigned cosys, unsigned codata)
{
    struct ilG_context_msg *msg = calloc(1, sizeof(struct ilG_context_msg));
    msg->type = ILG_DEL_COORDS;
    msg->value.coords.parent = self.id;
    msg->value.coords.cosys = cosys;
    msg->value.coords.codata = codata;
    ilG_context_queue_produce(self.context->queue, msg);
}

void ilG_handle_setViewCoords(ilG_handle self, ilG_cosysid cosys)
{
    struct ilG_context_msg *msg = calloc(1, sizeof(struct ilG_context_msg));
    msg->type = ILG_VIEW_COORDS;
    msg->value.coords.parent = self.id;
    msg->value.coords.cosys = cosys;
    ilG_context_queue_produce(self.context->queue, msg);
}

void ilG_handle_addRenderer(ilG_handle self, ilG_handle node)
{
    struct ilG_context_msg *msg = calloc(1, sizeof(struct ilG_context_msg));
    msg->type = ILG_ADD_RENDERER;
    msg->value.renderer.parent = self.id;
    msg->value.renderer.child = node.id;
    ilG_context_queue_produce(self.context->queue, msg);
}

void ilG_handle_delRenderer(ilG_handle self, ilG_handle node)
{
    struct ilG_context_msg *msg = calloc(1, sizeof(struct ilG_context_msg));
    msg->type = ILG_DEL_RENDERER;
    msg->value.renderer.parent = self.id;
    msg->value.renderer.child = node.id;
    ilG_context_queue_produce(self.context->queue, msg);
}

void ilG_handle_addLight(ilG_handle self, struct ilG_light light)
{
    struct ilG_context_msg *msg = calloc(1, sizeof(struct ilG_context_msg));
    msg->type = ILG_ADD_LIGHT;
    msg->value.light.parent = self.id;
    msg->value.light.child = light;
    ilG_context_queue_produce(self.context->queue, msg);
}

void ilG_handle_delLight(ilG_handle self, struct ilG_light light)
{
    struct ilG_context_msg *msg = calloc(1, sizeof(struct ilG_context_msg));
    msg->type = ILG_DEL_LIGHT;
    msg->value.light.parent = self.id;
    msg->value.light.child = light;
    ilG_context_queue_produce(self.context->queue, msg);
}

void ilG_handle_message(ilG_handle self, int type, il_value data)
{
    struct ilG_context_msg *msg = calloc(1, sizeof(struct ilG_context_msg));
    msg->type = ILG_MESSAGE;
    msg->value.message.node = self.id;
    msg->value.message.type = type;
    msg->value.message.data = data;
    ilG_context_queue_produce(self.context->queue, msg);
}
