#include "graphics/renderer.h"

#include <assert.h>

#include "graphics/context.h"
#include "graphics/material.h"
#include "asset/mesh.h"
#include "graphics/mesh.h"
#include "graphics/tex.h"
#include "graphics/transform.h"
#include "util/log.h"

void ilG_heightmap_free(ilG_heightmap *hm)
{
    ilG_renderman_delMaterial(hm->rm, hm->mat);
}

static void heightmap_free(void *ptr)
{
    ilG_heightmap_free(ptr);
}

void ilG_heightmap_draw(ilG_heightmap *hm, il_mat mvp, il_mat imt)
{
    ilG_tex_bind(&hm->height);
    ilG_tex_bind(&hm->normal);
    ilG_tex_bind(&hm->color);
    ilG_mesh_bind(&hm->mesh);
    ilG_material *shader = ilG_renderman_findMaterial(hm->rm, hm->mat);
    ilG_material_bind(shader);
    ilG_material_bindMatrix(shader, hm->mvp, mvp);
    ilG_material_bindMatrix(shader, hm->imt, imt);
    glUniform2f(hm->size, hm->w, hm->h);
    ilG_mesh_draw(&hm->mesh);
}

static void heightmap_draw(void *ptr, ilG_rendid id, il_mat **mats, const unsigned *objects, unsigned num_mats)
{
    (void)id, (void)objects, (void)num_mats;
    ilG_heightmap *self = ptr;
    assert(num_mats == 1);
    ilG_heightmap_draw(self, mats[0][0], mats[1][0]);
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
    height.unit = 0;
    normal.unit = 1;
    color.unit = 2;
    hm->height = height;
    hm->normal = normal;
    hm->color = color;

    ilG_tex_build(&hm->height);
    ilG_tex_build(&hm->normal);
    ilG_tex_build(&hm->color);

    ilG_material mat;
    ilG_material_init(&mat);
    ilG_material_name(&mat, "Heightmap Shader");
    ilG_material_arrayAttrib(&mat, ILG_MESH_POS, "in_Position");
    ilG_material_arrayAttrib(&mat, ILG_MESH_TEX, "in_Texcoord");
    ilG_material_textureUnit(&mat, 0, "height_tex");
    ilG_material_textureUnit(&mat, 1, "normal_tex");
    ilG_material_textureUnit(&mat, 2, "ambient_tex");
    ilG_material_fragData(&mat, ILG_CONTEXT_NORMAL, "out_Normal");
    ilG_material_fragData(&mat, ILG_CONTEXT_ALBEDO, "out_Albedo");
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
    if (ILG_MESH_ERROR & ilG_mesh_build(&hm->mesh)) {
        return false;
    }

    return true;
}

static bool heightmap_build(void *ptr, ilG_rendid id, ilG_renderman *rm, ilG_buildresult *out)
{
    (void)id;
    ilG_heightmap *self = ptr;
    if (!ilG_heightmap_build(self, rm, self->w, self->h, self->height, self->normal, self->color, &out->error)) {
        return false;
    }
    int *types = malloc(sizeof(int) * 2);
    types[0] = ILG_MVP;
    types[1] = ILG_INVERSE | ILG_MODEL | ILG_TRANSPOSE;
    *out = (ilG_buildresult) {
        .free = heightmap_free,
        .draw = heightmap_draw,
        .types = types,
        .num_types = 2,
        .obj = self,
        .name = strdup("Heightmap")
    };
    return true;
}

ilG_builder ilG_heightmap_builder(ilG_heightmap *hm, unsigned w, unsigned h,
                                  ilG_tex height, ilG_tex normal, ilG_tex color)
{
    hm->w = w;
    hm->h = h;
    hm->height = height;
    hm->normal = normal;
    hm->color = color;
    return ilG_builder_wrap(hm, heightmap_build);
}
