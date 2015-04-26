#include "graphics/renderer.h"

#include "asset/image.h"
#include "graphics/context.h"
#include "util/log.h"

typedef struct ilG_grabber {
    void (*cb)(ilA_img img, void *user);
    void *user;
    ilG_renderman *rm;
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
    ilA_img img;
    ilA_imgerr res = ilA_img_alloc(&img, self->context->height, self->context->width, ILA_IMG_U8, ILA_IMG_RGB);
    if (res) {
        il_error("Screenshot failed: %s", ilA_img_strerror(res));
        return;
    }
    glBindTexture(GL_TEXTURE_RECTANGLE, tgl_fbo_getTex(&self->context->accum, 0));
    glGetTexImage(GL_TEXTURE_RECTANGLE, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data);
    self->cb(img, self->user);
    ilG_renderman_delRenderer(self->rm, id);
    ilA_img_free(img);
}

static bool grabber_build(void *obj, ilG_rendid id, ilG_renderman *rm, ilG_buildresult *out)
{
    (void)id;
    ilG_grabber *self = obj;
    self->rm = rm;
    *out = (ilG_buildresult) {
        .free = grabber_free,
        .update = grabber_update,
        .draw = NULL,
        .view = NULL,
        .types = NULL,
        .num_types = 0,
        .obj = self,
        .name = strdup("Screenshotter")
    };
    return true;
}

ilG_builder ilG_grabber_builder(ilG_context *context, void (*cb)(ilA_img res, void *user), void *user)
{
    ilG_grabber *self = calloc(1, sizeof(ilG_grabber));
    self->context = context;
    self->cb = cb;
    self->user = user;
    return ilG_builder_wrap(self, grabber_build);
}
