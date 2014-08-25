#include "screenshot.h"

#include "asset/image.h"
#include "graphics/context.h"
#include "util/log.h"

typedef struct ilG_grabber {
    void (*cb)(ilA_img *img, void *user);
    void *user;
    ilG_context *context;
} ilG_grabber;

static void grabber_free(void *obj)
{
    ilG_grabber *self = obj;
    free(self);
}

static void grabber_update(void *obj, ilG_rendid id)
{
    ilG_grabber *self = obj;
    size_t size = self->context->width * self->context->height * 3;
    void *buf = malloc(size);
    glBindTexture(GL_TEXTURE_RECTANGLE, tgl_fbo_getTex(&self->context->fb, ILG_CONTEXT_ACCUM));
    glGetTexImage(GL_TEXTURE_RECTANGLE, 0, GL_RGB, GL_UNSIGNED_BYTE, buf);
    ilA_img *img = ilA_img_frombuf(buf, self->context->width, self->context->height, 0, ILA_IMG_RGB);
    self->cb(img, self->user);
    ilG_context_delRenderer(self->context, id);
}

static bool grabber_build(void *obj, ilG_rendid id, ilG_context *context, ilG_buildresult *out)
{
    (void)id;
    ilG_grabber *self = obj;
    self->context = context;
    *out = (ilG_buildresult) {
        .free = grabber_free,
        .update = grabber_update,
        .draw = NULL,
        .view = NULL,
        .types = NULL,
        .num_types = 0,
        .obj = self
    };
    return true;
}

ilG_builder ilG_grabber_builder(void (*cb)(ilA_img *res, void *user), void *user)
{
    ilG_grabber *self = calloc(1, sizeof(ilG_grabber));
    self->cb = cb;
    self->user = user;
    return ilG_builder_wrap(self, grabber_build);
}
