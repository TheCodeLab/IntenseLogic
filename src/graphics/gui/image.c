#include "frame.h"

#include <GL/glew.h>

#include "graphics/material.h"
#include "asset/asset.h"
#include "graphics/arrayattrib.h"
#include "graphics/fragdata.h"
#include "graphics/textureunit.h"
#include "graphics/context.h"
#include "graphics/gui/quad.h"
#include "graphics/texture.h"
#include "util/assert.h"

struct image_ctx {
    ilG_texture *tex;
    GLint pos_loc[2];
};

static void image_draw(ilG_gui_frame *self, ilG_gui_rect where)
{
    struct image_ctx *ctx = il_base_get(self, "il.graphics.gui.image.ctx", NULL, NULL);
    ilG_material *shader = il_base_get(self->context, "il.graphics.gui.image.shader", NULL, NULL);
    if (!shader) {
        shader = ilG_material_new();
        ilG_material_vertex(shader, IL_ASSET_READFILE("gui_image.vert"));
        ilG_material_fragment(shader, IL_ASSET_READFILE("gui_image.frag"));
        ilG_material_name(shader, "GUI Image");
        ilG_material_arrayAttrib(shader, ILG_ARRATTR_POSITION, "in_Position");
        ilG_material_fragData(shader, ILG_FRAGDATA_ACCUMULATION, "out_Color");
        ilG_material_textureUnit(shader, ILG_TUNIT_COLOR0, "tex");
        // TODO: Uniform for screen space position
        if (ilG_material_link(shader, self->context)) {
            return;
        }
        ctx->pos_loc[0] = glGetUniformLocation(shader->program, "pos1");
        ctx->pos_loc[1] = glGetUniformLocation(shader->program, "pos2");
        il_base_set(self->context, "il.graphics.gui.image.shader", shader, sizeof(ilG_material), IL_OBJECT);
    }
    ilG_bindable_swap(&self->context->materialb, (void**)&self->context->material, shader);
    ilG_bindable_swap(&self->context->drawableb, (void**)&self->context->drawable, ilG_quad(self->context));
    ilG_bindable_swap(&self->context->textureb, (void**)&self->context->texture, ctx->tex);
    glUniform2f(ctx->pos_loc[0], where.a.x / (float)self->context->width, where.a.y / (float)self->context->height);
    glUniform2f(ctx->pos_loc[1], where.b.x / (float)self->context->width, where.b.y / (float)self->context->height);

    ilG_bindable_action(self->context->materialb, self->context->material);
    ilG_bindable_action(self->context->textureb,  self->context->texture);
    ilG_bindable_action(self->context->drawableb, self->context->drawable);
}

void ilG_gui_frame_image(ilG_gui_frame *self, ilG_texture *tex)
{
    il_return_on_fail(self && tex);
    struct image_ctx *ctx = il_base_get(self, "il.graphics.gui.image.ctx", NULL, NULL);
    if (!ctx) {
        ctx = calloc(1, sizeof(struct image_ctx));
        il_base_set(self, "il.graphics.gui.image.ctx", ctx, sizeof(struct image_ctx), IL_VOID);
    }
    if (ctx->tex) {
        il_unref(ctx->tex);
    }
    ctx->tex = il_ref(tex);
    self->draw = image_draw;
}

