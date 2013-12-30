#include "frame.h"

#include <GL/glew.h>

#include "util/log.h"
#include "graphics/gui/quad.h"
#include "graphics/drawable3d.h"
#include "graphics/bindable.h"
#include "graphics/context.h"
#include "graphics/material.h"
#include "graphics/arrayattrib.h"
#include "graphics/fragdata.h"
#include "graphics/glutil.h"

static ilG_material *get_shader(ilG_context *context)
{
    ilG_material *mtl;
    if ((mtl = il_base_get(&context->base, "il.graphics.gui.frame.shader", NULL, NULL))) {
        return mtl;
    }
    mtl = ilG_material_new();
    ilG_material_name(mtl, "GUI Color Fill");
    ilG_material_vertex_file(mtl, "colorfill.vert");
    ilG_material_fragment_file(mtl, "colorfill.frag");
    ilG_material_arrayAttrib(mtl, ILG_ARRATTR_POSITION, "in_Position");
    ilG_material_fragData(mtl, ILG_FRAGDATA_ACCUMULATION, "out_Color");
    if (ilG_material_link(mtl, context)) {
        return NULL;
    }
    il_base_set(&context->base, "il.graphics.gui.frame.shader", mtl, 0, IL_OBJECT|IL_LOCAL_BIT);
    GLuint *col = malloc(sizeof(GLuint)),
           *pos = malloc(sizeof(GLuint)); 
    *col = glGetUniformLocation(mtl->program, "color");
    *pos = glGetUniformLocation(mtl->program, "position");
    il_base_set(&context->base, "il.graphics.gui.frame.color_loc", col, sizeof(GLuint), IL_VOID|IL_LOCAL_BIT);
    il_base_set(&context->base, "il.graphics.gui.frame.pos_loc", pos, sizeof(GLuint), IL_VOID|IL_LOCAL_BIT);
    return mtl;
}

static void filler_draw(ilG_gui_frame *self, ilG_gui_rect where)
{
    ilG_testError("Unknown");
    ilG_drawable3d *quad = ilG_quad(self->context);
    ilG_material *shader = get_shader(self->context);
    float *col = il_base_get(&self->base, "il.graphics.gui.frame.fillcolor", NULL, NULL);
    GLuint *col_loc = il_base_get(&self->context->base, "il.graphics.gui.frame.color_loc", NULL, NULL),
           *pos_loc = il_base_get(&self->context->base, "il.graphics.gui.frame.pos_loc", NULL, NULL);
    if (!col_loc || !pos_loc) {
        il_error("Could not retrieve shader information");
        return;
    }
    ilG_bindable_swap(&self->context->drawableb, (void**)&self->context->drawable, quad);
    ilG_bindable_swap(&self->context->materialb, (void**)&self->context->material, shader);
    ilG_bindable_action(self->context->materialb, self->context->material);
    glUniform4fv(*col_loc, 1, col);
    float w = self->context->width, h = self->context->height;
    glUniform4f(*pos_loc, where.a.x/w, where.a.y/h, where.b.x/w, where.b.y/h);
    ilG_bindable_action(self->context->drawableb, self->context->drawable);
    ilG_testError("filler_draw");
}

void ilG_gui_frame_filler(ilG_gui_frame *self, float col[4])
{
    if (!self->context) {
        il_error("No context set in filler");
        return;
    }
    il_base_set(&self->base, "il.graphics.gui.frame.fillcolor", col, 4, IL_FLOAT|IL_ARRAY_BIT);
    self->draw = filler_draw;
}

