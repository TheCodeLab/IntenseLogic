#include "graphics/renderer.h"

#include "graphics/arrayattrib.h"
#include "graphics/material.h"
#include "graphics/tex.h"
#include "graphics/transform.h"
#include "tgl/tgl.h"

void ilG_skybox_free(ilG_skybox *skybox)
{
    ilG_renderman_delMaterial(skybox->rm, skybox->mat);
}

void ilG_skybox_draw(ilG_skybox *skybox, il_mat vp)
{
    ilG_material *mat = ilG_renderman_findMaterial(skybox->rm, skybox->mat);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    ilG_shape_bind(skybox->box);
    ilG_material_bind(mat);
    ilG_material_bindMatrix(mat, ilG_material_getLoc(mat, "mat"), vp);

    ilG_tex_bind(&skybox->texture);
    ilG_shape_draw(skybox->box);
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
}

static void sky_view(void *ptr, ilG_rendid id, il_mat *mats)
{
    (void)id;
    ilG_skybox *self = ptr;

    ilG_skybox_draw(self, mats[0]);
    tgl_check("Unknown");
}

bool ilG_skybox_build(ilG_skybox *skybox, ilG_renderman *rm, ilG_tex skytex, ilG_shape *box, char **error)
{
    memset(skybox, 0, sizeof(*skybox));
    skybox->rm = rm;
    skybox->texture = skytex;
    skybox->texture.unit = 0;
    skybox->box = box;
    ilG_tex_build(&skybox->texture);

    ilG_material mat[1];
    ilG_material_init(mat);
    ilG_material_name(mat, "Skybox Shader");
    ilG_material_arrayAttrib(mat, ILG_ARRATTR_POSITION, "in_Position");
    ilG_material_arrayAttrib(mat, ILG_ARRATTR_TEXCOORD, "in_Texcoord");
    ilG_material_textureUnit(mat, 0, "skytex");
    ilG_material_fragData(mat, ILG_GBUFFER_NORMAL, "out_Normal");
    ilG_material_fragData(mat, ILG_GBUFFER_ALBEDO, "out_Color");
    if (!ilG_renderman_addMaterialFromFile(rm, *mat, "skybox.vert", "skybox.frag", &skybox->mat, error)) {
        return false;
    }

    return true;
}

static bool sky_build(void *ptr, ilG_rendid id, ilG_renderman *rm, ilG_buildresult *out)
{
    (void)id;
    ilG_skybox *self = ptr;
    ilG_tex skytex = self->texture;
    if (!ilG_skybox_build(self, rm, skytex, self->box, &out->error)) {
        return false;
    }

    int *types = malloc(1 * sizeof(int));
    types[0] = ILG_VIEW_R | ILG_PROJECTION;
    out->view = sky_view;
    out->types = types;
    out->num_types = 1;
    out->obj = self;
    out->name = strdup("Skybox");
    return true;
}

ilG_builder ilG_skybox_builder(ilG_skybox *skybox, ilG_tex skytex, ilG_shape *box)
{
    skybox->texture = skytex;
    skybox->box = box;
    return ilG_builder_wrap(skybox, sky_build);
}
