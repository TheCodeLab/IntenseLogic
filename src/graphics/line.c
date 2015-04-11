#include "line.h"

#include "tgl/tgl.h"
#include "graphics/context.h"
#include "graphics/material.h"
#include "graphics/fragdata.h"
#include "graphics/renderer.h"
#include "graphics/transform.h"

typedef struct ilG_line {
    float col[3];
    float *verts;
    unsigned count;
    ilG_matid mat;
    ilG_renderman *rm;
    tgl_vao vao;
    GLuint vbo;
    GLint mvp_loc, col_loc;
} ilG_line;

static void line_free(void *obj)
{
    ilG_line *self = obj;
    glDeleteBuffers(1, &self->vbo);
    tgl_vao_free(&self->vao);
    ilG_renderman_delMaterial(self->rm, self->mat);
}

static void line_draw(void *obj, ilG_rendid id, il_mat **mats, const unsigned *objects, unsigned num_mats)
{
    (void)id, (void)objects;
    ilG_line *self = obj;
    tgl_vao_bind(&self->vao);
    glBindBuffer(GL_ARRAY_BUFFER, self->vbo);
    ilG_material *mat = ilG_renderman_findMaterial(self->rm, self->mat);
    ilG_material_bind(mat);
    glUniform3fv(self->col_loc, 1, self->col);
    for (unsigned i = 0; i < num_mats; i++) {
        ilG_material_bindMatrix(mat, self->mvp_loc, mats[0][i]);
        glDrawArrays(GL_LINES, 0, self->count);
    }
}

static bool line_build(void *obj, ilG_rendid id, ilG_renderman *rm, ilG_buildresult *out)
{
    (void)id;
    ilG_line *self = obj;
    self->rm = rm;

    ilG_material m[1];
    ilG_material_init(m);
    ilG_material_name(m, "Line Segment Shader");
    ilG_material_arrayAttrib(m, 0, "in_Position");
    ilG_material_fragData(m, ILG_FRAGDATA_ACCUMULATION, "out_Ambient");
    if (!ilG_material_vertex_file(m, "line.vert", &out->error)) {
        return false;
    }
    if (!ilG_material_fragment_file(m, "line.frag", &out->error)) {
        return false;
    }
    if (!ilG_material_link(m, &out->error)) {
        return false;
    }
    self->mat = ilG_renderman_addMaterial(self->rm, *m);
    self->mvp_loc = ilG_material_getLoc(m, "mvp");
    self->col_loc = ilG_material_getLoc(m, "col");

    tgl_vao_init(&self->vao);
    tgl_vao_bind(&self->vao);
    glGenBuffers(1, &self->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, self->vbo);
    glBufferData(GL_ARRAY_BUFFER, self->count * 3 * sizeof(float), self->verts, GL_STATIC_DRAW);
    free(self->verts);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    int *types = malloc(sizeof(int) * 1);
    types[0] = ILG_MVP;
    *out = (ilG_buildresult) {
        .free = line_free,
        .draw = line_draw,
        .types = types,
        .num_types = 1,
        .obj = obj,
        .name = strdup("Lines")
    };
    return true;
}

ilG_builder ilG_line_builder(unsigned num, float *verts, const float col[3])
{
    ilG_line *self = calloc(1, sizeof(ilG_line));
    memcpy(self->col, col, sizeof(self->col));
    self->verts = verts;
    self->count = num;
    return ilG_builder_wrap(self, line_build);
}
