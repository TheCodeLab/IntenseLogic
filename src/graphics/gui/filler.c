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
    ilG_material *mtl = il_table_mgetsp(&context->storage, "gui.frame.shader");
    if (mtl) {
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
    il_table_setsp(&context->storage, "gui.frame.shader", il_opaque(mtl, il_unref));
    GLuint *col = malloc(sizeof(GLuint)),
           *pos = malloc(sizeof(GLuint)); 
    *col = glGetUniformLocation(mtl->program, "color");
    *pos = glGetUniformLocation(mtl->program, "position");
    il_table_setsp(&context->storage, "gui.frame.color_loc", il_opaque(col, free));
    il_table_setsp(&context->storage, "gui.frame.pos_loc", il_opaque(pos, free));
    return mtl;
}

static void filler_draw(ilG_gui_frame *self, ilG_gui_rect where)
{
    ilG_testError("Unknown");
    ilG_drawable3d *quad = ilG_quad(self->context);
    ilG_material *shader = get_shader(self->context);
    const il_vector *col = il_table_getsp(&self->base.storage, "gui.frame.fillcolor");
    const GLuint *col_loc = il_table_getsp(&self->context->storage, "gui.frame.color_loc"),
                 *pos_loc = il_table_getsp(&self->context->storage, "gui.frame.pos_loc");
    if (!col_loc || !pos_loc) {
        il_error("Could not retrieve shader information");
        return;
    }
    ilG_bindable_swap(&self->context->drawableb, (void**)&self->context->drawable, quad);
    ilG_bindable_swap(&self->context->materialb, (void**)&self->context->material, shader);
    ilG_bindable_action(self->context->materialb, self->context->material);
    glUniform4f(*col_loc, il_vector_getf(col, 0), 
                          il_vector_getf(col, 1),
                          il_vector_getf(col, 2),
                          il_vector_getf(col, 3));
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
    il_table_sets(&self->base.storage, "gui.frame.fillcolor", il_value_vectorl(4, col[0], col[1], col[2], col[3]));
    self->draw = filler_draw;
}

