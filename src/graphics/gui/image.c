#include "frame.h"

#include <GL/glew.h>

#include "graphics/material.h"
#include "graphics/arrayattrib.h"
#include "graphics/fragdata.h"
#include "graphics/textureunit.h"
#include "graphics/context.h"
#include "graphics/gui/quad.h"
#include "graphics/tex.h"
#include "util/ilassert.h"
#include "graphics/glutil.h"

struct image {
    GLint pos_loc[2];
    ilG_tex tex;
    bool premultiply, first, have_tex;
    ilG_material *shader;
};

static void image_draw(ilG_gui_frame *self, ilG_gui_rect where)
{
    struct image *img = il_table_mgetsp(&self->base.storage, "gui.image");

    ilG_testError("Unknown");
    glEnable(GL_BLEND);
    if (img->premultiply) {
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    ilG_bindable_swap(&self->context->materialb, (void**)&self->context->material, img->shader);
    ilG_bindable_swap(&self->context->drawableb, (void**)&self->context->drawable, ilG_quad(self->context));
    glUniform2f(img->pos_loc[0], where.a.x / (float)self->context->width, where.a.y / (float)self->context->height);
    glUniform2f(img->pos_loc[1], where.b.x / (float)self->context->width, where.b.y / (float)self->context->height);

    ilG_bindable_action(self->context->materialb, self->context->material);
    ilG_tex_bind(&img->tex);
    ilG_bindable_action(self->context->drawableb, self->context->drawable);
    glDisable(GL_BLEND);
    ilG_testError("image_draw");
}

static int image_build(ilG_gui_frame *self, ilG_context *context)
{
    (void)context;
    struct image *img = il_table_mgetsp(&self->base.storage, "gui.image");
    il_log("Build image %p", self);
    if (img->first && ilG_material_link(img->shader, self->context)) {
        return 0;
    }
    img->first = false;
    img->pos_loc[0] = glGetUniformLocation(img->shader->program, "pos1");
    img->pos_loc[1] = glGetUniformLocation(img->shader->program, "pos2");
    if (img->have_tex) {
        ilG_tex_build(&img->tex, self->context);
    }
    return 1;
}

static void image_message(ilG_gui_frame *self, int type, il_value v)
{
    (void)type;
    struct image *img = il_table_mgetsp(&self->base.storage, "gui.image");
    const il_vector *vec = il_value_tovec(&v);
    const ilG_tex *tex = il_vector_getp(vec, 0);
    int premultiplied = il_vector_geti(vec, 1);
    memcpy(&img->tex, tex, sizeof(ilG_tex));
    ilG_tex_build(&img->tex, self->context);
    img->premultiply = premultiplied;
}

void ilG_gui_frame_image(ilG_gui_frame *self)
{
    il_return_on_fail(self);
    struct image *img = il_table_mgetsp(&self->base.storage, "gui.image");
    if (img) {
        return;
    }
    img = calloc(1, sizeof(struct image));
    img->first = true;
    ilG_material *shader = img->shader = ilG_material_new();
    ilG_material_vertex_file(shader, "gui_image.vert");
    ilG_material_fragment_file(shader, "gui_image.frag");
    ilG_material_name(shader, "GUI Image");
    ilG_material_arrayAttrib(shader, ILG_ARRATTR_POSITION, "in_Position");
    ilG_material_fragData(shader, ILG_FRAGDATA_ACCUMULATION, "out_Color");
    ilG_material_textureUnit(shader, ILG_TUNIT_COLOR0, "tex");

    il_table_setsp(&self->base.storage, "gui.image", il_opaque(img, free));
    self->draw = image_draw;
    self->build = image_build;
    self->message = image_message;
}

void ilG_gui_frame_image_setTex(ilG_gui_frame *self, struct ilG_tex *tex, int premultiplied)
{
    if (self->context) {
        ilG_tex *mem = calloc(1, sizeof(ilG_tex));
        memcpy(mem, tex, sizeof(*mem));
        ilG_context_message(self->context, ilG_gui_frame_wrap(self), 0, il_value_vectorl(2, il_vopaque(mem, free), il_value_int(premultiplied)));
    } else {
        struct image *img = il_table_mgetsp(&self->base.storage, "gui.image");
        memcpy(&img->tex, tex, sizeof(ilG_tex));
        img->have_tex = true;
    }
}

