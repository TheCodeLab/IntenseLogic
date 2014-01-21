#include "frame.h"

#include <GL/glew.h>

#include "graphics/material.h"
#include "graphics/arrayattrib.h"
#include "graphics/fragdata.h"
#include "graphics/textureunit.h"
#include "graphics/context.h"
#include "graphics/gui/quad.h"
#include "graphics/texture.h"
#include "util/ilassert.h"
#include "graphics/glutil.h"

struct shader_ctx {
    GLint pos_loc[2];
};

static ilG_material *get_shader(ilG_gui_frame *self)
{
    ilG_material *shader = il_table_mgetsp(&self->context->base.storage, "gui.image.shader");
    if (shader) {
        return shader;
    }
    shader = ilG_material_new();
    ilG_material_vertex_file(shader, "gui_image.vert");
    ilG_material_fragment_file(shader, "gui_image.frag");
    ilG_material_name(shader, "GUI Image");
    ilG_material_arrayAttrib(shader, ILG_ARRATTR_POSITION, "in_Position");
    ilG_material_fragData(shader, ILG_FRAGDATA_ACCUMULATION, "out_Color");
    ilG_material_textureUnit(shader, ILG_TUNIT_COLOR0, "tex");
    // TODO: Uniform for screen space position
    if (ilG_material_link(shader, self->context)) {
        return NULL;
    }
    il_table_setsp(&self->context->base.storage, "gui.image.shader", il_opaque(shader, il_unref));
    return shader;
}

static void image_draw(ilG_gui_frame *self, ilG_gui_rect where)
{
    ilG_texture *tex = il_table_mgetsp(&self->base.storage, "gui.image.tex");
    int premultiplied = il_table_getsb(&self->base.storage, "gui.image.premultiply");
    ilG_material *shader = get_shader(self);
    struct shader_ctx *shader_ctx = il_table_mgetsp(&shader->base.storage, "gui.image.shaderctx");

    ilG_testError("Unknown");
    if (!shader_ctx) {
        shader_ctx = calloc(1, sizeof(struct shader_ctx));
        shader_ctx->pos_loc[0] = glGetUniformLocation(shader->program, "pos1");
        shader_ctx->pos_loc[1] = glGetUniformLocation(shader->program, "pos2");
        il_table_setsp(&shader->base.storage, "gui.image.shaderctx", il_opaque(shader_ctx, free));
    }

    glEnable(GL_BLEND);
    if (premultiplied) {
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    ilG_bindable_swap(&self->context->materialb, (void**)&self->context->material, shader);
    ilG_bindable_swap(&self->context->drawableb, (void**)&self->context->drawable, ilG_quad(self->context));
    ilG_bindable_swap(&self->context->textureb, (void**)&self->context->texture, tex);
    glUniform2f(shader_ctx->pos_loc[0], where.a.x / (float)self->context->width, where.a.y / (float)self->context->height);
    glUniform2f(shader_ctx->pos_loc[1], where.b.x / (float)self->context->width, where.b.y / (float)self->context->height);

    ilG_bindable_action(self->context->materialb, self->context->material);
    ilG_bindable_action(self->context->textureb,  self->context->texture);
    ilG_bindable_action(self->context->drawableb, self->context->drawable);
    glDisable(GL_BLEND);
    ilG_testError("image_draw");
}

void ilG_gui_frame_image(ilG_gui_frame *self, ilG_texture *tex, int premultiplied)
{
    il_return_on_fail(self && tex);
    il_table_setsp(&self->base.storage, "gui.image.tex", il_opaque(il_ref(tex), il_unref));
    il_table_setsb(&self->base.storage, "gui.image.premultiplied", premultiplied);
    self->draw = image_draw;
}

