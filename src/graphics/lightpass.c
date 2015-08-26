#include "graphics/renderer.h"

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

enum ilG_light_type {
    ILG_POINT,
    ILG_SUN,
};

typedef struct ilG_lights ilG_lights;
struct ilG_lights {
    enum ilG_light_type type;
    ilG_renderman *rm;
    ilG_context *context;
    GLint lights_size, mvp_size, lights_offset[3], mvp_offset[1], color_loc, radius_loc, mvp_loc, mv_loc, ivp_loc, size_loc, fovsquared_loc;
    ilG_matid mat;
    ilG_shape *ico;
    tgl_vao vao;
    tgl_quad quad;
    const char *file;
};

enum {
    TEX_DEPTH,
    TEX_NORMAL,
    TEX_ALBEDO,
    TEX_REFRACTION,
    TEX_GLOSS
};

static void lights_free(void *ptr)
{
    ilG_lights *self = ptr;
    ilG_renderman_delMaterial(self->rm, self->mat);
    tgl_vao_free(&self->vao);
    tgl_quad_free(&self->quad);
    free(self);
}

static void lights_draw(void *ptr, ilG_rendid id, il_mat **mats, const unsigned *objects, unsigned num_mats)
{
    (void)id, (void)objects;
    ilG_lights *self = ptr;
    ilG_context *context = self->context;
    const bool point = self->type == ILG_POINT;
    ilG_material *mat = ilG_renderman_findMaterial(self->rm, self->mat);
    ilG_material_bind(mat);
    glActiveTexture(GL_TEXTURE0 + TEX_DEPTH);
    tgl_fbo_bindTex(&context->gbuffer, ILG_CONTEXT_DEPTH);
    glActiveTexture(GL_TEXTURE0 + TEX_NORMAL);
    tgl_fbo_bindTex(&context->gbuffer, ILG_CONTEXT_NORMAL);
    glActiveTexture(GL_TEXTURE0 + TEX_ALBEDO);
    tgl_fbo_bindTex(&context->gbuffer, ILG_CONTEXT_ALBEDO);
    glActiveTexture(GL_TEXTURE0 + TEX_REFRACTION);
    tgl_fbo_bindTex(&context->gbuffer, ILG_CONTEXT_REFRACTION);
    glActiveTexture(GL_TEXTURE0 + TEX_GLOSS);
    tgl_fbo_bindTex(&context->gbuffer, ILG_CONTEXT_GLOSS);
    tgl_fbo_bind(&context->accum, TGL_FBO_RW);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE);
    glDepthMask(GL_FALSE);
    if (point) {
        //glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        glCullFace(GL_FRONT);
        ilG_shape_bind(self->ico);
    } else {
        glDisable(GL_CULL_FACE);
        tgl_vao_bind(&self->vao);
    }
    tgl_check("Unknown");

    ilG_renderer *r = ilG_renderman_findRenderer(self->rm, id);
    for (unsigned i = 0; i < num_mats; i++) {
        ilG_material_bindMatrix(mat, self->ivp_loc, mats[0][i]);
        ilG_material_bindMatrix(mat, self->mv_loc,  mats[1][i]);
        ilG_material_bindMatrix(mat, self->mvp_loc, mats[2][i]);
        glUniform2f(self->size_loc, context->width, context->height);
        ilG_light *l = &r->lights.data[i];
        il_vec3 col = l->color;
        glUniform3f(self->color_loc, col.x, col.y, col.z);
        glUniform1f(self->radius_loc, l->radius);
        glUniform1f(self->fovsquared_loc, context->fovsquared);
        if (point) {
            ilG_shape_draw(self->ico);
        } else {
            tgl_quad_draw_once(&self->quad);
        }
    }

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    tgl_check("Error drawing lights");
}

static bool lights_build(void *ptr, ilG_rendid id, ilG_renderman *rm, ilG_buildresult *out)
{
    (void)id;
    ilG_lights *self = ptr;
    self->rm = rm;

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
    if (!ilG_renderman_addMaterialFromFile(rm, m, self->file,
                                           self->context->msaa? "light_msaa.frag" : "light.frag",
                                           &self->mat, &out->error)) {
        return false;
    }
    GLuint prog = ilG_renderman_findMaterial(rm, self->mat)->program;
    self->color_loc     = glGetUniformLocation(prog, "color");
    self->radius_loc    = glGetUniformLocation(prog, "radius");
    self->mvp_loc       = glGetUniformLocation(prog, "mvp");
    self->mv_loc        = glGetUniformLocation(prog, "mv");
    self->ivp_loc       = glGetUniformLocation(prog, "ivp");
    self->size_loc      = glGetUniformLocation(prog, "size");
    self->fovsquared_loc= glGetUniformLocation(prog, "fovsquared");

    self->ico = ilG_icosahedron(rm);
    tgl_vao_init(&self->vao);
    tgl_vao_bind(&self->vao);
    tgl_quad_init(&self->quad, ILG_ARRATTR_POSITION);

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

ilG_builder ilG_pointlight_builder(ilG_context *context)
{
    ilG_lights *self = calloc(1, sizeof(ilG_lights));
    self->context = context;
    self->type = ILG_POINT;
    self->file = "light.vert";
    return ilG_builder_wrap(self, lights_build);
}

ilG_builder ilG_sunlight_builder(ilG_context *context)
{
    ilG_lights *self = calloc(1, sizeof(ilG_lights));
    self->context = context;
    self->type = ILG_SUN;
    self->file = "id2d.vert";
    return ilG_builder_wrap(self, lights_build);
}
