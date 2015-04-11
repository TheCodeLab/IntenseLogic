#include "skyboxpass.h"

#include "graphics/arrayattrib.h"
#include "graphics/context.h"
#include "graphics/fragdata.h"
#include "graphics/material.h"
#include "graphics/shape.h"
#include "graphics/tex.h"
#include "graphics/transform.h"
#include "tgl/tgl.h"

typedef struct ilG_skybox {
    ilG_renderman *rm;
    ilG_matid mat;
    ilG_shape *box;
    ilG_tex texture;
    GLuint vp_loc;
} ilG_skybox;

static void sky_free(void *ptr)
{
    ilG_skybox *self = ptr;
    ilG_renderman_delMaterial(self->rm, self->mat);
}

static void sky_view(void *ptr, ilG_rendid id, il_mat *mats)
{
    (void)id;
    ilG_skybox *self = ptr;

    ilG_material *mat = ilG_renderman_findMaterial(self->rm, self->mat);

    tgl_check("Unknown");
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    ilG_shape_bind(self->box);
    ilG_material_bind(mat);
    ilG_material_bindMatrix(mat, self->vp_loc, mats[0]);

    ilG_tex_bind(&self->texture);
    ilG_shape_draw(self->box);
    glEnable(GL_DEPTH_TEST);
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);
}

static bool sky_build(void *ptr, ilG_rendid id, ilG_renderman *rm, ilG_buildresult *out)
{
    (void)id;
    ilG_skybox *self = ptr;
    self->rm = rm;
    ilG_tex_build(&self->texture);

    ilG_material m;
    ilG_material_init(&m);
    ilG_material_name(&m, "Skybox Shader");
    ilG_material_arrayAttrib(&m, ILG_ARRATTR_POSITION, "in_Position");
    ilG_material_arrayAttrib(&m, ILG_ARRATTR_TEXCOORD, "in_Texcoord");
    ilG_material_textureUnit(&m, 0, "skytex");
    ilG_material_fragData(&m, ILG_FRAGDATA_ACCUMULATION, "out_Color");
    ilG_material_fragData(&m, ILG_FRAGDATA_NORMAL, "out_Normal");
    ilG_material_fragData(&m, ILG_FRAGDATA_DIFFUSE, "out_Diffuse");
    ilG_material_fragData(&m, ILG_FRAGDATA_SPECULAR, "out_Specular");
    if (!ilG_material_vertex_file(&m, "skybox.vert", &out->error)) {
        return false;
    }
    if (!ilG_material_fragment_file(&m, "skybox.frag", &out->error)) {
        return false;
    }
    if (!ilG_material_link(&m, &out->error)) {
        return false;
    }
    self->vp_loc = ilG_material_getLoc(&m, "mat");
    self->mat = ilG_renderman_addMaterial(rm, m);

    self->box = ilG_box(rm);

    int *types = malloc(1 * sizeof(int));
    types[0] = ILG_VIEW_R | ILG_PROJECTION;
    *out = (ilG_buildresult) {
        .free = sky_free,
        .view = sky_view,
        .update = NULL,
        .draw = NULL,
        .types = types,
        .num_types = 1,
        .obj = self,
        .name = strdup("Skybox")
    };
    return true;
}

ilG_builder ilG_skybox_builder(ilG_tex skytex)
{
    ilG_skybox *self = calloc(1, sizeof(ilG_skybox));
    self->texture = skytex;
    self->texture.unit = 0;
    return ilG_builder_wrap(self, sky_build);
}
