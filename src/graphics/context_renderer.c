#include "graphics/context.h"

#include "util/log.h"
#include "util/logger.h"
#include "input/input.h"
#include "graphics/glutil.h"

void queue_free(struct ilG_context_queue *queue);

static void context_free(void *ptr) 
{
    ilG_context *self = ptr;

    self->complete = 0;

    il_value nil = il_value_nil();
    ilE_handler_fire(self->destroy, &nil);
    il_value_free(nil);

    free(self->texunits);
    IL_FREE(self->renderers);
    ilE_unregister(self->tick, self->tick_id);
    ilE_handler_destroy(self->tick);
    ilE_handler_destroy(self->resize);
    ilE_handler_destroy(self->close);
    queue_free(self->queue);

    SDL_GL_DeleteContext(self->context);
    SDL_DestroyWindow(self->window);
    free(self);
}

static void context_draw(void *ptr)
{
    ilG_context *self = ptr;

    ilG_testError("Unkown");
    for (unsigned i = 0; i < self->renderers.length; i++) {
        il_debug("Rendering %s", ilG_renderer_getName(&self->renderers.data[i]));
        self->renderers.data[i].vtable->draw(self->renderers.data[i].obj);
        ilG_testError("In %s", ilG_renderer_getName(&self->renderers.data[i]));
    }
}

void ilG_registerInputBackend(ilG_context *ctx);
static int context_build(void *ptr, ilG_context *ctx)
{
    (void)ctx;
    ilG_context *self = ptr;

    for (unsigned i = 0; i < self->renderers.length; i++) {
        self->renderers.data[i].vtable->build(self->renderers.data[i].obj, self);
    }

    return 1;
}

static il_table *context_get_storage(void *ptr)
{
    ilG_context *self = ptr;
    return &self->storage;
}

static bool context_get_complete(const void *ptr)
{
    const ilG_context *self = ptr;
    return self->complete;
}

static void context_add_renderer(void *ptr, ilG_renderer r)
{
    ilG_context *self = ptr;
    IL_APPEND(self->renderers, r);
}

static void context_del_renderer(void *ptr, ilG_renderer node)
{
    ilG_context *self = ptr;
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
    il_error("Renderer %s<%p> does not own %s<%p>", ilG_context_renderer.name, ptr, node.vtable->name, node.obj);
}

static void context_message(void *ptr, int type, il_value *v)
{
    switch (type) {
    case 1:
        context_add_renderer(ptr, *(ilG_renderer*)il_value_tomvoid(v));
        break;
    case 2:
        context_del_renderer(ptr, *(ilG_renderer*)il_value_tomvoid(v));
        break;
    default:
        il_error("No such message %i for %s<%p>", type, ilG_context_renderer.name, ptr);
    }
}

static void context_push_msg(void *ptr, int type, il_value v)
{
    ilG_context *self = ptr;
    if (self->window) { // little bit of a hack to tell if the thread is running
        ilG_context_message(self, ilG_context_wrap(self), type, v);
    } else {
        context_message(ptr, type, &v);
        il_value_free(v);
    }
}

const ilG_renderable ilG_context_renderer = {
    .free = context_free,
    .draw = context_draw,
    .build = context_build,
    .get_storage = context_get_storage,
    .get_complete = context_get_complete,
    .add_renderer = 1,
    .del_renderer = 2,
    .message = context_message,
    .push_msg = context_push_msg,
    .name = "Context"
};

