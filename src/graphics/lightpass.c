#include "graphics/renderer.h"

#include <assert.h>

#include "graphics/arrayattrib.h"
#include "graphics/context.h"
#include "graphics/material.h"
#include "graphics/shape.h"
#include "graphics/transform.h"
#include "math/matrix.h"
#include "math/vector.h"
#include "tgl/tgl.h"
#include "util/array.h"
#include "util/log.h"

enum {
    TEX_DEPTH,
    TEX_NORMAL,
    TEX_ALBEDO,
    TEX_REFRACTION,
    TEX_GLOSS
};

void ilG_lighting_free(ilG_lighting *lighting)
{
    ilG_renderman_delMaterial(lighting->rm, lighting->mat);
    tgl_vao_free(&lighting->vao);
    tgl_quad_free(&lighting->quad);
}

static void lights_free(void *ptr)
{
    ilG_lighting_free(ptr);
}

void ilG_lighting_draw(ilG_lighting *lighting, il_mat **mats, ilG_light *lights, size_t count)
{
    ilG_material *mat = ilG_renderman_findMaterial(lighting->rm, lighting->mat);
    const bool point = lighting->type == ILG_POINT;
    ilG_material_bind(mat);
    glActiveTexture(GL_TEXTURE0 + TEX_DEPTH);
    tgl_fbo_bindTex(lighting->gbuffer, ILG_CONTEXT_DEPTH);
    glActiveTexture(GL_TEXTURE0 + TEX_NORMAL);
    tgl_fbo_bindTex(lighting->gbuffer, ILG_CONTEXT_NORMAL);
    glActiveTexture(GL_TEXTURE0 + TEX_ALBEDO);
    tgl_fbo_bindTex(lighting->gbuffer, ILG_CONTEXT_ALBEDO);
    glActiveTexture(GL_TEXTURE0 + TEX_REFRACTION);
    tgl_fbo_bindTex(lighting->gbuffer, ILG_CONTEXT_REFRACTION);
    glActiveTexture(GL_TEXTURE0 + TEX_GLOSS);
    tgl_fbo_bindTex(lighting->gbuffer, ILG_CONTEXT_GLOSS);
    tgl_fbo_bind(lighting->accum, TGL_FBO_RW);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE);
    glDepthMask(GL_FALSE);
    if (point) {
        //glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        glCullFace(GL_FRONT);
        ilG_shape_bind(lighting->ico);
    } else {
        glDisable(GL_CULL_FACE);
        tgl_vao_bind(&lighting->vao);
    }
    tgl_check("Unknown");

    for (unsigned i = 0; i < count; i++) {
        ilG_material_bindMatrix(mat, lighting->ivp_loc, mats[0][i]);
        ilG_material_bindMatrix(mat, lighting->mv_loc,  mats[1][i]);
        ilG_material_bindMatrix(mat, lighting->mvp_loc, mats[2][i]);
        glUniform2f(lighting->size_loc, lighting->width, lighting->height);
        ilG_light *l = &lights[i];
        il_vec3 col = l->color;
        glUniform3f(lighting->color_loc, col.x, col.y, col.z);
        glUniform1f(lighting->radius_loc, l->radius);
        glUniform1f(lighting->fovsquared_loc, lighting->fovsquared);
        if (point) {
            ilG_shape_draw(lighting->ico);
        } else {
            tgl_quad_draw_once(&lighting->quad);
        }
    }

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}

static void lights_draw(void *ptr, ilG_rendid id, il_mat **mats, const unsigned *objects, unsigned num_mats)
{
    (void)id, (void)objects;
    ilG_lighting *self = ptr;

    ilG_renderer *r = ilG_renderman_findRenderer(self->rm, id);
    assert(r->lights.length == num_mats);
    ilG_lighting_draw(self, mats, r->lights.data, num_mats);
    tgl_check("Error drawing lights");
}

bool ilG_lighting_build(ilG_lighting *lighting, ilG_renderman *rm,
                        ilG_light_type type, bool msaa, char **error)
{
    memset(lighting, 0, sizeof(*lighting));
    lighting->rm = rm;

    const char *file = type == ILG_POINT? "light.vert" : "id2d.vert";

    ilG_material m;
    ilG_material_init(&m);
    ilG_material_name(&m, "Deferred Lighting Shader");
    ilG_material_arrayAttrib(&m, ILG_ARRATTR_POSITION, "in_Position");
    ilG_material_textureUnit(&m, TEX_DEPTH, "depth");
    ilG_material_textureUnit(&m, TEX_NORMAL, "normal");
    ilG_material_textureUnit(&m, TEX_ALBEDO, "albedo");
    ilG_material_textureUnit(&m, TEX_REFRACTION, "refraction");
    ilG_material_textureUnit(&m, TEX_GLOSS, "gloss");
    ilG_material_fragData(&m, 0, "out_Color");
    if (!ilG_renderman_addMaterialFromFile(rm, m, file,
                                           msaa? "light_msaa.frag" : "light.frag",
                                           &lighting->mat, error)) {
        return false;
    }
    GLuint prog = ilG_renderman_findMaterial(rm, lighting->mat)->program;
    lighting->color_loc     = glGetUniformLocation(prog, "color");
    lighting->radius_loc    = glGetUniformLocation(prog, "radius");
    lighting->mvp_loc       = glGetUniformLocation(prog, "mvp");
    lighting->mv_loc        = glGetUniformLocation(prog, "mv");
    lighting->ivp_loc       = glGetUniformLocation(prog, "ivp");
    lighting->size_loc      = glGetUniformLocation(prog, "size");
    lighting->fovsquared_loc= glGetUniformLocation(prog, "fovsquared");

    lighting->ico = ilG_icosahedron(rm);
    tgl_vao_init(&lighting->vao);
    tgl_vao_bind(&lighting->vao);
    tgl_quad_init(&lighting->quad, ILG_ARRATTR_POSITION);

    return true;
}

static bool lights_build(void *ptr, ilG_rendid id, ilG_renderman *rm, ilG_buildresult *out)
{
    (void)id;
    ilG_lighting *self = ptr;
    if (!ilG_lighting_build(self, rm, self->type, self->msaa, &out->error)) {
        return false;
    }

    int *types = malloc(sizeof(int) * 3);
    types[0] = ILG_INVERSE | ILG_VIEW_R | ILG_PROJECTION;
    types[1] = ILG_MODEL_T | ILG_VIEW_T;
    types[2] = ILG_MODEL_T | ILG_VP;
    *out = (ilG_buildresult) {
        .free = lights_free,
        .update = NULL,
        .draw = lights_draw,
        .view = NULL,
        .types = types,
        .num_types = 3,
        .obj = self,
        .name = strdup("Deferred Shading")
    };
    return true;
}

ilG_builder ilG_lighting_builder(ilG_lighting *self, bool msaa, ilG_light_type type)
{
    self->type = type;
    self->msaa = msaa;
    return ilG_builder_wrap(self, lights_build);
}
