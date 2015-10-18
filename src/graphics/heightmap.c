#include "graphics/renderer.h"

#include <assert.h>

#include "graphics/material.h"
#include "asset/mesh.h"
#include "graphics/mesh.h"
#include "graphics/tex.h"
#include "graphics/transform.h"
#include "util/log.h"

enum {
    TEX_HEIGHT,
    TEX_NORMAL,
    TEX_COLOR
};

void ilG_heightmap_free(ilG_heightmap *hm)
{
    ilG_renderman_delMaterial(hm->rm, hm->mat);
    ilG_tex_free(&hm->height);
    ilG_tex_free(&hm->normal);
    ilG_tex_free(&hm->color);
}

void ilG_heightmap_draw(ilG_heightmap *hm, il_mat mvp, il_mat imt)
{
    ilG_tex_bind(&hm->height, TEX_HEIGHT);
    ilG_tex_bind(&hm->normal, TEX_NORMAL);
    ilG_tex_bind(&hm->color, TEX_COLOR);
    ilG_mesh_bind(&hm->mesh);
    ilG_material *shader = ilG_renderman_findMaterial(hm->rm, hm->mat);
    ilG_material_bind(shader);
    ilG_material_bindMatrix(shader, hm->mvp, mvp);
    ilG_material_bindMatrix(shader, hm->imt, imt);
    glUniform2f(hm->size, hm->w, hm->h);
    ilG_mesh_draw(&hm->mesh);
}

bool ilG_heightmap_build(ilG_heightmap *hm, ilG_renderman *rm, unsigned w, unsigned h,
                         ilG_tex height, ilG_tex normal, ilG_tex color, char **error)
{
    memset(hm, 0, sizeof(*hm));
    hm->rm = rm;
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

    hm->w = w;
    hm->h = h;
    hm->height = height;
    hm->normal = normal;
    hm->color = color;

    ilG_material mat;
    ilG_material_init(&mat);
    ilG_material_name(&mat, "Heightmap Shader");
    ilG_material_arrayAttrib(&mat, ILG_MESH_POS, "in_Position");
    ilG_material_arrayAttrib(&mat, ILG_MESH_TEX, "in_Texcoord");
    ilG_material_textureUnit(&mat, TEX_HEIGHT, "height_tex");
    ilG_material_textureUnit(&mat, TEX_NORMAL, "normal_tex");
    ilG_material_textureUnit(&mat, TEX_COLOR, "ambient_tex");
    ilG_material_fragData(&mat, ILG_GBUFFER_NORMAL, "out_Normal");
    ilG_material_fragData(&mat, ILG_GBUFFER_ALBEDO, "out_Albedo");
    if (!ilG_renderman_addMaterialFromFile(hm->rm, mat, "heightmap.vert", "heightmap.frag", &hm->mat, error)) {
        return false;
    }
    ilG_material *mat2 = ilG_renderman_findMaterial(rm, hm->mat);
    hm->mvp = ilG_material_getLoc(mat2, "mvp");
    hm->imt = ilG_material_getLoc(mat2, "imt");
    hm->size = ilG_material_getLoc(mat2, "size");

    if (!ilG_mesh_init(&hm->mesh, mesh)) {
        ilA_mesh_free(mesh);
        return false;
    }
    ilA_mesh_free(mesh);

    return true;
}
