#include "renderer.h"

#include "context.h"

typedef struct ilG_ambient {
    ilG_matid mat;
    GLuint col_loc;
    ilG_renderman *rm;
    ilG_context *context;
    tgl_quad quad;
    tgl_vao vao;
    il_vec3 *color;
} ilG_ambient;

static void ambient_free(void *ptr)
{
    (void)ptr;
}

static void ambient_update(void *ptr, ilG_rendid id)
{
    (void)id;
    ilG_ambient *self = ptr;

    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE);
    glDepthMask(GL_FALSE);

    glActiveTexture(GL_TEXTURE0);
    tgl_fbo_bindTex(&self->context->gbuffer, ILG_CONTEXT_DIFFUSE);
    tgl_fbo_bind(&self->context->accum, TGL_FBO_RW);
    ilG_material_bind(ilG_renderman_findMaterial(self->rm, self->mat));
    tgl_vao_bind(&self->vao);
    glUniform3f(self->col_loc, self->color->x, self->color->y, self->color->z);
    tgl_quad_draw_once(&self->quad);

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}

static bool ambient_build(void *ptr, ilG_rendid id, ilG_renderman *rm, ilG_buildresult *out)
{
    (void)id;
    ilG_ambient *self = ptr;

    self->rm = rm;

    ilG_material m;
    ilG_material_init(&m);
    ilG_material_name(&m, "Ambient Lighting");
    ilG_material_arrayAttrib(&m, 0, "in_Texcoord");
    ilG_material_fragData(&m, 0, "out_Color");
    ilG_material_textureUnit(&m, 0, "tex");
    if (!ilG_renderman_addMaterialFromFile(self->rm, m, "id2d.vert", "ambient.frag", &self->mat, &out->error)) {
        return false;
    }
    ilG_material *mat = ilG_renderman_findMaterial(self->rm, self->mat);
    self->col_loc = ilG_material_getLoc(mat, "color");

    tgl_vao_init(&self->vao);
    tgl_vao_bind(&self->vao);
    tgl_quad_init(&self->quad, 0);

    out->free = ambient_free;
    out->update = ambient_update;
    out->obj = ptr;
    out->name = strdup("Ambient Lighting");
    return true;
}

ilG_builder ilG_ambient_builder(ilG_context *context, il_vec3 *color)
{
    ilG_ambient *self = calloc(1, sizeof(ilG_ambient));
    self->context = context;
    self->color = color;
    return ilG_builder_wrap(self, ambient_build);
}
