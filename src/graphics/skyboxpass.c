#include "skyboxpass.h"

#include "graphics/context.h"
#include "graphics/bindable.h"
#include "graphics/material.h"
#include "graphics/arrayattrib.h"
#include "graphics/textureunit.h"
#include "graphics/shape.h"
#include "graphics/texture.h"
#include "graphics/fragdata.h"

struct ilG_skybox {
    ilG_context *context;
    ilG_material *material;
    ilG_texture *texture;
};

static ilG_material *skybox_shader(ilG_context *context)
{
    ilG_material *self = ilG_material_new();
    ilG_material_vertex_file(self, "skybox.vert");
    ilG_material_fragment_file(self, "skybox.frag");
    ilG_material_name(self, "Skybox Shader");
    ilG_material_arrayAttrib(self, ILG_ARRATTR_POSITION, "in_Position");
    ilG_material_arrayAttrib(self, ILG_ARRATTR_TEXCOORD, "in_Texcoord");
    ilG_material_textureUnit(self, ILG_TUNIT_COLOR0, "skytex");
    ilG_material_matrix(self, ILG_VIEW_R | ILG_PROJECTION, "mat");
    ilG_material_fragData(self, ILG_FRAGDATA_ACCUMULATION, "out_Color");
    ilG_material_fragData(self, ILG_FRAGDATA_NORMAL, "out_Normal");
    ilG_material_fragData(self, ILG_FRAGDATA_DIFFUSE, "out_Diffuse");
    ilG_material_fragData(self, ILG_FRAGDATA_SPECULAR, "out_Specular");
    if (ilG_material_link(self, context)) {
        il_unref(self);
        return NULL;
    }
    return self;
}

static void sky_run(void *ptr)
{
    ilG_skybox *self = ptr;
    ilG_context *context = self->context;

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    ilG_bindable_swap(&context->drawableb, (void**)&context->drawable, ilG_box(context));
    ilG_bindable_swap(&context->materialb, (void**)&context->material, self->material);
    ilG_bindable_swap(&context->textureb,  (void**)&context->texture,  self->texture);

    ilG_bindable_action(context->materialb, context->material);
    ilG_bindable_action(context->textureb,  context->texture);
    ilG_bindable_action(context->drawableb, context->drawable);
    glEnable(GL_DEPTH_TEST);
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);
}

static int sky_track(void *ptr, ilG_renderer renderer)
{
    (void)renderer, (void)ptr;
    return 0;
}

const ilG_stagable ilG_skybox_stage = {
    .run = sky_run,
    .track = sky_track,
    .name = "Skybox"
};

ilG_skybox *ilG_skybox_new(ilG_context *context, ilG_texture *skytex)
{
    ilG_skybox *self = calloc(1, sizeof(ilG_skybox));
    self->context = context;
    self->material = skybox_shader(context);
    self->texture = skytex;
    return self;
}

