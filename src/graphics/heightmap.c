#include "graphics/renderer.h"

#include "graphics/context.h"
#include "graphics/material.h"
#include "asset/mesh.h"
#include "graphics/fragdata.h"
#include "graphics/mesh.h"
#include "graphics/tex.h"
#include "graphics/transform.h"
#include "util/log.h"

typedef struct ilG_heightmap {
    ilG_tex height, normal, color;
    ilG_matid mat;
    ilA_mesh *source;
    ilG_mesh mesh;
    unsigned w,h;
    ilG_renderman *rm;
    GLenum mvp, imt, size;
} ilG_heightmap;

static void heightmap_free(void *ptr)
{
    ilG_heightmap *self = ptr;
    ilG_renderman_delMaterial(self->rm, self->mat);
    free(self);
}

static void heightmap_draw(void *ptr, ilG_rendid id, il_mat **mats, const unsigned *objects, unsigned num_mats)
{
    (void)id, (void)objects;
    ilG_heightmap *self = ptr;
    ilG_tex_bind(&self->height);
    ilG_tex_bind(&self->normal);
    ilG_tex_bind(&self->color);
    ilG_mesh_bind(&self->mesh);
    ilG_material *shader = ilG_renderman_findMaterial(self->rm, self->mat);
    ilG_material_bind(shader);
    for (unsigned i = 0; i < num_mats; i++) {
        ilG_material_bindMatrix(shader, self->mvp, mats[0][i]);
        ilG_material_bindMatrix(shader, self->imt, mats[1][i]);
        glUniform2f(self->size, self->w, self->h);
        ilG_mesh_draw(&self->mesh);
    }
}

static bool heightmap_build(void *ptr, ilG_rendid id, ilG_renderman *rm, ilG_buildresult *out)
{
    (void)id;
    ilG_heightmap *self = ptr;
    self->rm = rm;
    ilG_tex_build(&self->height);
    ilG_tex_build(&self->normal);
    ilG_tex_build(&self->color);

    ilG_material mat;
    ilG_material_init(&mat);
    ilG_material_name(&mat, "Heightmap Shader");
    ilG_material_arrayAttrib(&mat, ILG_MESH_POS, "in_Position");
    ilG_material_arrayAttrib(&mat, ILG_MESH_TEX, "in_Texcoord");
    ilG_material_textureUnit(&mat, 0, "height_tex");
    ilG_material_textureUnit(&mat, 1, "normal_tex");
    ilG_material_textureUnit(&mat, 2, "ambient_tex");
    ilG_material_fragData(&mat, ILG_FRAGDATA_ACCUMULATION, "out_Ambient");
    ilG_material_fragData(&mat, ILG_FRAGDATA_NORMAL, "out_Normal");
    ilG_material_fragData(&mat, ILG_FRAGDATA_DIFFUSE, "out_Diffuse");
    ilG_material_fragData(&mat, ILG_FRAGDATA_SPECULAR, "out_Specular");
    if (!ilG_renderman_addMaterialFromFile(self->rm, mat, "heightmap.vert", "heightmap.frag", &self->mat, &out->error)) {
        return false;
    }
    ilG_material *mat2 = ilG_renderman_findMaterial(rm, self->mat);
    self->mvp = ilG_material_getLoc(mat2, "mvp");
    self->imt = ilG_material_getLoc(mat2, "imt");
    self->size = ilG_material_getLoc(mat2, "size");

    if (!ilG_mesh_init(&self->mesh, self->source)) {
        ilA_mesh_free(self->source);
        return false;
    }
    ilA_mesh_free(self->source);
    if (ILG_MESH_ERROR & ilG_mesh_build(&self->mesh)) {
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

ilG_builder ilG_heightmap_builder(unsigned w, unsigned h, ilG_tex height, ilG_tex normal, ilG_tex color)
{
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

    ilG_heightmap *self = calloc(1, sizeof(ilG_heightmap));
    self->w = w;
    self->h = h;
    height.unit = 0;
    normal.unit = 1;
    color.unit = 2;
    self->height = height;
    self->normal = normal;
    self->color = color;
    self->source = mesh;

    return ilG_builder_wrap(self, heightmap_build);
}
