#include "skyboxpass.h"

#include "graphics/context.h"
#include "graphics/bindable.h"
#include "graphics/material.h"
#include "graphics/arrayattrib.h"
#include "graphics/textureunit.h"
#include "graphics/shape.h"
#include "graphics/tex.h"
#include "graphics/fragdata.h"

struct ilG_skybox {
    ilG_context *context;
    ilG_material *material;
    ilG_tex texture;
    bool complete;
    il_table storage;
};

static void sky_free(void *ptr)
{
    ilG_skybox *self = ptr;
    il_unref(self->material);
    il_table_free(self->storage);
}

static void sky_draw(void *ptr)
{
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

static int sky_build(void *ptr, ilG_context *context)
{
    ilG_skybox *self = ptr;
    self->context = context;
    ilG_tex_build(&self->texture, context);
    if (ilG_material_link(self->material, context)) {
        return 0;
    }
    return self->complete = 1;
}

static il_table *sky_get_storage(void *ptr)
{
    ilG_skybox *self = ptr;
    return &self->storage;
}

static bool sky_get_complete(const void *ptr)
{
    const ilG_skybox *self = ptr;
    return self->complete;
}

const ilG_renderable ilG_skybox_renderer = {
    .free = sky_free,
    .draw = sky_draw,
    .build = sky_build,
    .get_storage = sky_get_storage,
    .get_complete = sky_get_complete,
    .add_positionable = NULL,
    .add_renderer = NULL,
    .name = "Skybox"
};

ilG_skybox *ilG_skybox_new(ilG_tex skytex)
{
    ilG_skybox *self = calloc(1, sizeof(ilG_skybox));

    ilG_material *shader = ilG_material_new();
    ilG_material_vertex_file(shader, "skybox.vert");
    ilG_material_fragment_file(shader, "skybox.frag");
    ilG_material_name(shader, "Skybox Shader");
    ilG_material_arrayAttrib(shader, ILG_ARRATTR_POSITION, "in_Position");
    ilG_material_arrayAttrib(shader, ILG_ARRATTR_TEXCOORD, "in_Texcoord");
    ilG_material_textureUnit(shader, ILG_TUNIT_COLOR0, "skytex");
    ilG_material_matrix(shader, ILG_VIEW_R | ILG_PROJECTION, "mat");
    ilG_material_fragData(shader, ILG_FRAGDATA_ACCUMULATION, "out_Color");
    ilG_material_fragData(shader, ILG_FRAGDATA_NORMAL, "out_Normal");
    ilG_material_fragData(shader, ILG_FRAGDATA_DIFFUSE, "out_Diffuse");
    ilG_material_fragData(shader, ILG_FRAGDATA_SPECULAR, "out_Specular");

    self->material = shader;
    self->texture = skytex;
    return self;
}

