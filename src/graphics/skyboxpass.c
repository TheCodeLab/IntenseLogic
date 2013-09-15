#include "skyboxpass.h"

#include "graphics/context.h"
#include "graphics/bindable.h"
#include "graphics/material.h"
#include "graphics/arrayattrib.h"
#include "graphics/textureunit.h"
#include "graphics/shape.h"
#include "graphics/texture.h"
#include "graphics/fragdata.h"

static ilG_material *skybox_shader(ilG_context *context)
{
    ilG_material *self = il_base_get(context, "il.graphics.skybox.shader", NULL, NULL);
    if (self) {
        return self;
    }
    self = ilG_material_new();
    ilG_material_vertex_file(self, "skybox.vert");
    ilG_material_fragment_file(self, "skybox.frag");
    ilG_material_name(self, "Skybox Shader");
    ilG_material_arrayAttrib(self, ILG_ARRATTR_POSITION, "in_Position");
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
    il_base_set(context, "il.graphics.skybox.shader", self, 0, IL_OBJECT);
    return self;
}

static void draw_sky(ilG_stage *self)
{
    ilG_context *context = self->context;

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    ilG_bindable_swap(&context->drawableb, (void**)&context->drawable, ilG_box(context));
    ilG_bindable_swap(&context->materialb, (void**)&context->material, skybox_shader(context));
    ilG_bindable_swap(&context->textureb,  (void**)&context->texture,  il_base_get(self, "il.graphics.shader.texture", NULL, NULL));

    ilG_bindable_action(context->materialb, context->material);
    ilG_bindable_action(context->textureb,  context->texture);
    ilG_bindable_action(context->drawableb, context->drawable);
    glEnable(GL_DEPTH_TEST);
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void ilG_skyboxpass(ilG_stage *self, ilG_texture *skytex)
{
    self->run = draw_sky;
    self->name = "Skybox";
    il_base_set(self, "il.graphics.shader.texture", skytex, 0, IL_OBJECT);
    skybox_shader(self->context);
}

