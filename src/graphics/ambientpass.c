#include "renderer.h"

void ilG_ambient_free(ilG_ambient *ambient)
{
    ilG_renderman_delMaterial(ambient->rm, ambient->mat);
    tgl_quad_free(&ambient->quad);
    tgl_vao_free(&ambient->vao);
}

enum {
    TEX_ALBEDO,
    TEX_EMISSION
};

void ilG_ambient_draw(ilG_ambient *ambient)
{
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE);
    glDepthMask(GL_FALSE);

    glActiveTexture(GL_TEXTURE0 + TEX_ALBEDO);
    tgl_fbo_bindTex(&ambient->rm->gbuffer, ILG_GBUFFER_ALBEDO);
    glActiveTexture(GL_TEXTURE0 + TEX_EMISSION);
    tgl_fbo_bindTex(&ambient->rm->gbuffer, ILG_GBUFFER_EMISSION);

    tgl_fbo_bind(&ambient->rm->accum, TGL_FBO_RW);
    ilG_material_bind(ilG_renderman_findMaterial(ambient->rm, ambient->mat));
    tgl_vao_bind(&ambient->vao);
    glUniform3f(ambient->col_loc, ambient->color.x, ambient->color.y, ambient->color.z);
    glUniform1f(ambient->fovsquared_loc, ambient->fovsquared);
    tgl_quad_draw_once(&ambient->quad);

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}

static void ambient_update(void *ptr, ilG_rendid id)
{
    (void)id;
    ilG_ambient *self = ptr;
    ilG_ambient_draw(self);
}

bool ilG_ambient_build(ilG_ambient *ambient, ilG_renderman *rm, char **error)
{
    memset(ambient, 0, sizeof(*ambient));
    ambient->rm = rm;
    ilG_material m;
    ilG_material_init(&m);
    ilG_material_name(&m, "Ambient Lighting");
    ilG_material_arrayAttrib(&m, 0, "in_Texcoord");
    ilG_material_fragData(&m, 0, "out_Color");
    ilG_material_textureUnit(&m, TEX_ALBEDO, "tex_Albedo");
    ilG_material_textureUnit(&m, TEX_EMISSION, "tex_Emission");
    if (!ilG_renderman_addMaterialFromFile(ambient->rm, m, "id2d.vert", "ambient.frag", &ambient->mat, error)) {
        return false;
    }
    ilG_material *mat = ilG_renderman_findMaterial(ambient->rm, ambient->mat);
    ambient->col_loc = ilG_material_getLoc(mat, "color");
    ambient->fovsquared_loc = ilG_material_getLoc(mat, "fovsquared");

    tgl_vao_init(&ambient->vao);
    tgl_vao_bind(&ambient->vao);
    tgl_quad_init(&ambient->quad, 0);

    return true;
}

static bool ambient_build(void *ptr, ilG_rendid id, ilG_renderman *rm, ilG_buildresult *out)
{
    (void)id, (void)rm;
    ilG_ambient *self = ptr;

    if (!ilG_ambient_build(self, rm, &out->error)) {
        return false;
    }

    out->update = ambient_update;
    out->obj = ptr;
    out->name = strdup("Ambient Lighting");

    return true;
}

ilG_builder ilG_ambient_builder(ilG_ambient *ambient)
{
    return ilG_builder_wrap(ambient, ambient_build);
}
