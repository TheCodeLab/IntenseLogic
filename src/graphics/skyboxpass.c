#include "skyboxpass.h"

#include "graphics/context.h"
#include "graphics/bindable.h"
#include "graphics/material.h"
#include "graphics/arrayattrib.h"
#include "graphics/shape.h"
#include "graphics/tex.h"
#include "graphics/fragdata.h"

typedef struct ilG_skybox {
    ilG_context *context;
    ilG_material *material;
    ilG_tex texture;
} ilG_skybox;

static void sky_free(void *ptr)
{
    ilG_skybox *self = ptr;
    il_unref(self->material);
}

static void sky_update(void *ptr, ilG_rendid id)
{
    (void)id;
    ilG_skybox *self = ptr;
    ilG_context *context = self->context;

    ilG_testError("Unknown");
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    ilG_bindable_swap(&context->drawableb, (void**)&context->drawable, ilG_box(context));
    ilG_bindable_swap(&context->materialb, (void**)&context->material, self->material);

    ilG_bindable_action(context->materialb, context->material);
    ilG_tex_bind(&self->texture);
    ilG_bindable_action(context->drawableb, context->drawable);
    glEnable(GL_DEPTH_TEST);
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);
}

static bool sky_build(void *ptr, ilG_rendid id, ilG_context *context, ilG_buildresult *out)
{
    ilG_skybox *self = ptr;
    self->context = context;
    ilG_tex_build(&self->texture, context);
    if (ilG_material_link(self->material, context)) {
        return false;
    }
    ilG_context_addName(context, id, "Skybox");
    *out = (ilG_buildresult) {
        .free = sky_free,
        .update = sky_update,
        .draw = NULL,
        .types = NULL,
        .num_types = 0,
        .obj = self
    };
    return true;
}

ilG_builder ilG_skybox_builder(ilG_tex skytex)
{
    ilG_skybox *self = calloc(1, sizeof(ilG_skybox));

    ilG_material *shader = ilG_material_new();
    ilG_material_vertex_file(shader, "skybox.vert");
    ilG_material_fragment_file(shader, "skybox.frag");
    ilG_material_name(shader, "Skybox Shader");
    ilG_material_arrayAttrib(shader, ILG_ARRATTR_POSITION, "in_Position");
    ilG_material_arrayAttrib(shader, ILG_ARRATTR_TEXCOORD, "in_Texcoord");
    ilG_material_textureUnit(shader, 0, "skytex");
    ilG_material_matrix(shader, ILG_VIEW_R | ILG_PROJECTION, "mat");
    ilG_material_fragData(shader, ILG_FRAGDATA_ACCUMULATION, "out_Color");
    ilG_material_fragData(shader, ILG_FRAGDATA_NORMAL, "out_Normal");
    ilG_material_fragData(shader, ILG_FRAGDATA_DIFFUSE, "out_Diffuse");
    ilG_material_fragData(shader, ILG_FRAGDATA_SPECULAR, "out_Specular");

    self->material = shader;
    self->texture = skytex;
    self->texture.unit = 0;
    return ilG_builder_wrap(self, sky_build);
}
