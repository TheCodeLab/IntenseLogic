#include "heightmap.h"

#include "graphics/context.h"
#include "graphics/drawable3d.h"
#include "graphics/material.h"
#include "asset/mesh.h"
#include "graphics/textureunit.h"
#include "graphics/fragdata.h"
#include "graphics/arrayattrib.h"
#include "graphics/mesh.h"

ilG_drawable3d *ilG_heightmap_new(ilG_context *context, unsigned w, unsigned h)
{
    (void)context;
    ilA_mesh *mesh = ilA_mesh_new(ILA_MESH_POSITION|ILA_MESH_TEXCOORD, w * h * 6);
    unsigned x, y, i = 0, j;
    
    mesh->mode = ILA_MESH_TRIANGLES;
    for (y = 1; y < h; y++) {
        for (x = 1; x < w; x++) {
            float pos[] = {
                x,  y,  0, 1,
                x,  y-1,0, 1,
                x-1,y,  0, 1,

                x,  y-1,0, 1,
                x-1,y-1,0, 1,
                x-1,y,  0, 1,
            };
            for (j = 0; j < 24; j += 4) {
                pos[j] /= w;
            }
            for (j = 1; j < 24; j += 4) {
                pos[j] /= h;
            }
            memcpy(mesh->position + i, pos, sizeof(pos));
            float tex[] = {
                x, y, 0, 0,
                x, y, 0, 0,
                x, y, 0, 0,

                x, y, 0, 0,
                x, y, 0, 0,
                x, y, 0, 0,
            };
            memcpy(mesh->texcoord + i, tex, sizeof(tex));
            i += 6;
        }
    }
    ilG_drawable3d *dr = ilG_mesh(mesh);
    ilA_mesh_free(mesh);
    il_table_sets(&dr->base.storage, "heightmap.size", il_value_vectorl(2, il_value_int(w), il_value_int(h)));
    return dr;
}

static void width_uniform(ilG_material *self, GLint location, void *user)
{
    (void)user;
    il_vector *size = il_value_tovec(il_table_gets(&self->context->drawable->base.storage, "heightmap.size"));
    if (!size) {
        il_error("Heightmap shader combined with non-heightmap drawable");
        return;
    }
    glUniform2f(location, il_vector_getf(size, 0), il_vector_getf(size, 1));
}

ilG_material *ilG_heightmap_shader(ilG_context *context)
{
    ilG_material *mat = il_value_tovoid(il_table_gets(&context->base.storage, "heightmap.shader"));
    if (mat) {
        return mat;
    }
    mat = ilG_material_new();
    ilG_material_vertex_file(mat, "heightmap.vert");
    ilG_material_fragment_file(mat, "heightmap.frag");
    ilG_material_name(mat, "Heightmap Shader");
    ilG_material_arrayAttrib(mat, ILG_ARRATTR_POSITION, "in_Position");
    ilG_material_arrayAttrib(mat, ILG_ARRATTR_TEXCOORD, "in_Texcoord");
    ilG_material_textureUnit(mat, ILG_TUNIT_HEIGHT0, "height_tex");
    ilG_material_textureUnit(mat, ILG_TUNIT_NORMAL0, "normal_tex");
    ilG_material_textureUnit(mat, ILG_TUNIT_COLOR0,  "ambient_tex");
    ilG_material_fragData(mat, ILG_FRAGDATA_ACCUMULATION, "out_Ambient");
    ilG_material_fragData(mat, ILG_FRAGDATA_NORMAL, "out_Normal");
    ilG_material_fragData(mat, ILG_FRAGDATA_DIFFUSE, "out_Diffuse");
    ilG_material_fragData(mat, ILG_FRAGDATA_SPECULAR, "out_Specular");
    ilG_material_matrix(mat, ILG_MVP, "mvp");
    ilG_material_matrix(mat, ILG_INVERSE | ILG_MODEL | ILG_TRANSPOSE, "imt");
    ilG_material_bindFunc(mat, width_uniform, NULL, "size");
    if (ilG_material_link(mat, context)) {
        il_unref(mat);
        return NULL;
    }
    il_table_sets(&context->base.storage, "heightmap.shader", il_value_opaque(mat, il_unref));

    return mat;
}

