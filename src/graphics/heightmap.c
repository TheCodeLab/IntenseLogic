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
    ilA_mesh *mesh = ilA_mesh_new(ILA_MESH_POSITION, w * h * 6);
    unsigned x, y, i = 0;
    
    mesh->mode = ILA_MESH_TRIANGLES;
    for (y = 1; y < h; y++) {
        for (x = 1; x < w; x++) {
            float patch[] = {
                x/(float)w,     y/(float)h,     0, 1,
                x/(float)w,     (y-1)/(float)h, 0, 1,
                (x-1)/(float)w, y/(float)h,     0, 1,

                x/(float)w,     (y-1)/(float)h, 0, 1,
                (x-1)/(float)w, (y-1)/(float)h, 0, 1,
                (x-1)/(float)w, y/(float)h,     0, 1,
            };
            memcpy(mesh->position + i, patch, sizeof(patch));
            i += 6;
        }
    }
    ilG_drawable3d *dr = ilG_mesh(mesh);
    ilA_mesh_free(mesh);
    return dr;
}

ilG_material *ilG_heightmap_shader(ilG_context *context)
{
    ilG_material *mat = il_base_get(context, "il.graphics.heightmap.shader", NULL, NULL);
    if (mat) {
        return mat;
    }
    mat = ilG_material_new();
    ilG_material_vertex_file(mat, "heightmap.vert");
    ilG_material_fragment_file(mat, "heightmap.frag");
    ilG_material_name(mat, "Heightmap Shader");
    ilG_material_arrayAttrib(mat, ILG_ARRATTR_POSITION, "in_Position");
    ilG_material_textureUnit(mat, ILG_TUNIT_HEIGHT0, "height_tex");
    ilG_material_fragData(mat, ILG_FRAGDATA_ACCUMULATION, "out_Ambient");
    ilG_material_fragData(mat, ILG_FRAGDATA_NORMAL, "out_Normal");
    ilG_material_fragData(mat, ILG_FRAGDATA_DIFFUSE, "out_Diffuse");
    ilG_material_fragData(mat, ILG_FRAGDATA_SPECULAR, "out_Specular");
    ilG_material_matrix(mat, ILG_MVP, "mvp");
    if (ilG_material_link(mat, context)) {
        il_unref(mat);
        return NULL;
    }
    il_base_set(context, "il.graphics.heightmap.shader", mat, 0, IL_OBJECT);

    return mat;
}

