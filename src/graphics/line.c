#include "graphics/renderer.h"

#include "tgl/tgl.h"
#include "graphics/context.h"
#include "graphics/material.h"
#include "graphics/transform.h"

void ilG_wireframe_free(ilG_wireframe *wf)
{
    glDeleteBuffers(1, &wf->vbo);
    tgl_vao_free(&wf->vao);
    ilG_renderman_delMaterial(wf->rm, wf->mat);
}

void ilG_wireframe_draw(ilG_wireframe *wf, il_mat mvp, const float col[3])
{
    tgl_vao_bind(&wf->vao);
    glBindBuffer(GL_ARRAY_BUFFER, wf->vbo);
    ilG_material *mat = ilG_renderman_findMaterial(wf->rm, wf->mat);
    ilG_material_bind(mat);
    glUniform3fv(wf->col_loc, 1, col);
    ilG_material_bindMatrix(mat, wf->mvp_loc, mvp);
    glDrawArrays(GL_LINES, 0, wf->count);
}

bool ilG_wireframe_build(ilG_wireframe *wf, ilG_renderman *rm, char **error)
{
    wf->rm = rm;

    ilG_material m[1];
    ilG_material_init(m);
    ilG_material_name(m, "Line Segment Shader");
    ilG_material_arrayAttrib(m, 0, "in_Position");
    ilG_material_fragData(m, ILG_CONTEXT_ALBEDO, "out_Ambient");
    if (!ilG_renderman_addMaterialFromFile(rm, *m, "line.vert", "line.frag", &wf->mat, error)) {
        return false;
    }
    ilG_material *mat = ilG_renderman_findMaterial(rm, wf->mat);
    wf->mvp_loc = ilG_material_getLoc(mat, "mvp");
    wf->col_loc = ilG_material_getLoc(mat, "col");

    tgl_vao_init(&wf->vao);
    tgl_vao_bind(&wf->vao);
    glGenBuffers(1, &wf->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, wf->vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    return true;
}

void ilG_wireframe_data(ilG_wireframe *wf, const float *verts, size_t count)
{
    wf->count = count;
    glBindBuffer(GL_ARRAY_BUFFER, wf->vbo);
    glBufferData(GL_ARRAY_BUFFER, count * 3 * sizeof(float), verts, GL_STATIC_DRAW);
}
