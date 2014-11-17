#include "context.h"
#include "context-internal.h"

#include <limits.h>

#include "util/log.h"

/////////////////////////////////////////////////////////////////////////////
// Message wrappers

bool ilG_context_upload(ilG_context *self, void (*fn)(void*), void* ptr)
{
    ilG_context_msg msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = ILG_UPLOAD;
    msg.v.upload.cb = fn;
    msg.v.upload.ptr = ptr;
    ilG_context_queue_produce(self->queue, msg);
    return true;
}

bool ilG_context_resize(ilG_context *self, int w, int h)
{
    ilG_context_msg msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = ILG_RESIZE;
    msg.v.resize[0] = w;
    msg.v.resize[1] = h;
    ilG_context_queue_produce(self->queue, msg);
    return true;
}

char *strdup(const char*);
bool ilG_context_rename(ilG_context *self, const char *title)
{
    SDL_SetWindowTitle(self->window, title);
    return true;
}

void ilG_context_setNotifier(ilG_context *self, void (*fn)(il_value*), il_value val)
{
    self->client->notify = fn;
    self->client->user = val;
}

void ilG_context_message(ilG_context *self, ilG_rendid id, int type, il_value val)
{
    ilG_client_msg msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = ILG_CLIENT_MESSAGE;
    msg.v.message.id = id;
    msg.v.message.type = type;
    msg.v.message.data = val;
    ilG_client_queue_produce(self->client, msg);
}

unsigned ilG_context_addRenderer(ilG_context *self, ilG_rendid id, ilG_builder builder)
{
    ilG_buildresult b;
    memset(&b, 0, sizeof(ilG_buildresult));
    bool res = builder.build(builder.obj, id, self, &b);
    ilG_renderer r = (ilG_renderer) {
        .free = NULL,
        .children = {0,0,0},
        .lights = {0,0,0},
        .obj = 0,
        .view = 0,
        .stat = 0,
        .data = NULL
    };

    if (!res) {
        ilG_error e = (ilG_error) {id, b.error};
        il_error("Renderer failed: %s", b.error);
        IL_APPEND(self->manager.failed, e);
        ilG_client_msg msg;
        memset(&msg, 0, sizeof(msg));
        msg.type = ILG_FAILURE;
        msg.v.failure.id = id;
        msg.v.failure.msg = b.error;
        ilG_client_queue_produce(self->client, msg);
        return UINT_MAX;
    }

    r.free = b.free;
    r.data = b.obj;
    if (b.update) {
        ilG_statrenderer s = (ilG_statrenderer) {
            .update = b.update
        };
        r.stat = self->manager.statrenderers.length;
        IL_APPEND(self->manager.statrenderers, s);
    }
    if (b.view) {
        for (unsigned i = 0; i < b.num_types; i++) {
            if (b.types[i] & ILG_MODEL) {
                il_error("View renderers cannot have model transformations");
                return UINT_MAX;
            }
        }
        ilG_viewrenderer v = (ilG_viewrenderer) {
            .update = b.view,
            .coordsys = 0, // TODO: Select coord system
            .types = b.types,
            .num_types = b.num_types,
        };
        r.view = self->manager.viewrenderers.length;
        IL_APPEND(self->manager.viewrenderers, v);
    }
    if (b.draw) {
        ilG_objrenderer m = (ilG_objrenderer) {
            .draw = b.draw,
            .coordsys = 0, // TODO: Select coord system
            .types = b.types,
            .num_types = b.num_types
        };
        r.obj = self->manager.objrenderers.length;
        IL_APPEND(self->manager.objrenderers, m);
    }
    IL_APPEND(self->manager.renderers, r);
    IL_APPEND(self->manager.rendids, id);
    ilG_client_msg msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = ILG_READY;
    msg.v.ready = id;
    ilG_client_queue_produce(self->client, msg);
    return self->manager.renderers.length - 1;
}
