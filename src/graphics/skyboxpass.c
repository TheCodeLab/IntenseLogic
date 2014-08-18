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
    ilG_context *context;
    ilG_material material;
    ilG_shape *box;
    ilG_tex texture;
    GLuint vp_loc;
} ilG_skybox;

static void sky_free(void *ptr)
{
    ilG_skybox *self = ptr;
    ilG_material_free(&self->material);
}

static void sky_view(void *ptr, ilG_rendid id, il_mat *mats)
{
    (void)id;
    ilG_skybox *self = ptr;

    tgl_check("Unknown");
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    ilG_shape_bind(self->box);
    ilG_material_bind(&self->material);
    ilG_material_bindMatrix(&self->material, self->vp_loc, mats[0]);

    ilG_tex_bind(&self->texture);
    ilG_shape_draw(self->box);
    glEnable(GL_DEPTH_TEST);
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);
}

static bool sky_build(void *ptr, ilG_rendid id, ilG_context *context, ilG_buildresult *out)
{
    ilG_skybox *self = ptr;
    self->context = context;
    ilG_tex_build(&self->texture, context);
    if (ilG_material_link(&self->material, context)) {
        return false;
    }
    self->vp_loc = ilG_material_getLoc(&self->material, "mat");

    self->box = ilG_box(context);

    ilG_context_addName(context, id, "Skybox");
    int *types = malloc(1 * sizeof(int));
    types[0] = ILG_VIEW_R | ILG_PROJECTION;
    *out = (ilG_buildresult) {
        .free = sky_free,
        .view = sky_view,
        .update = NULL,
        .draw = NULL,
        .types = types,
        .num_types = 1,
        .obj = self
    };
    return true;
}

ilG_builder ilG_skybox_builder(ilG_tex skytex)
{
    ilG_skybox *self = calloc(1, sizeof(ilG_skybox));

    ilG_material_init(&self->material);
    ilG_material *shader = &self->material;
    ilG_material_vertex_file(shader, "skybox.vert");
    ilG_material_fragment_file(shader, "skybox.frag");
    ilG_material_name(shader, "Skybox Shader");
    ilG_material_arrayAttrib(shader, ILG_ARRATTR_POSITION, "in_Position");
    ilG_material_arrayAttrib(shader, ILG_ARRATTR_TEXCOORD, "in_Texcoord");
    ilG_material_textureUnit(shader, 0, "skytex");
    ilG_material_fragData(shader, ILG_FRAGDATA_ACCUMULATION, "out_Color");
    ilG_material_fragData(shader, ILG_FRAGDATA_NORMAL, "out_Normal");
    ilG_material_fragData(shader, ILG_FRAGDATA_DIFFUSE, "out_Diffuse");
    ilG_material_fragData(shader, ILG_FRAGDATA_SPECULAR, "out_Specular");

    self->texture = skytex;
    self->texture.unit = 0;
    return ilG_builder_wrap(self, sky_build);
}
