#include "graphics/context.h"
#include "graphics/floatspace.h"
#include "graphics/material.h"
#include "graphics/renderer.h"
#include "graphics/transform.h"
#include "graphics/tex.h"
#include "math/matrix.h"
#include "util/log.h"

#include "tgl/tgl.h"

static const float cube[] = {
    // front
    -1.0, -1.0,  1.0,
     1.0, -1.0,  1.0,
     1.0,  1.0,  1.0,
     1.0,  1.0,  1.0,
    -1.0,  1.0,  1.0,
    -1.0, -1.0,  1.0,
    // top
    -1.0,  1.0,  1.0,
     1.0,  1.0,  1.0,
     1.0,  1.0, -1.0,
     1.0,  1.0, -1.0,
    -1.0,  1.0, -1.0,
    -1.0,  1.0,  1.0,
    // back
     1.0, -1.0, -1.0,
    -1.0, -1.0, -1.0,
    -1.0,  1.0, -1.0,
    -1.0,  1.0, -1.0,
     1.0,  1.0, -1.0,
     1.0, -1.0, -1.0,
    // bottom
    -1.0, -1.0, -1.0,
     1.0, -1.0, -1.0,
     1.0, -1.0,  1.0,
     1.0, -1.0,  1.0,
    -1.0, -1.0,  1.0,
    -1.0, -1.0, -1.0,
    // left
    -1.0, -1.0, -1.0,
    -1.0, -1.0,  1.0,
    -1.0,  1.0,  1.0,
    -1.0,  1.0,  1.0,
    -1.0,  1.0, -1.0,
    -1.0, -1.0, -1.0,
    // right
     1.0, -1.0,  1.0,
     1.0, -1.0, -1.0,
     1.0,  1.0, -1.0,
     1.0,  1.0, -1.0,
     1.0,  1.0,  1.0,
     1.0, -1.0,  1.0,
};

static const float cube_n[] = {
    // front
     0.0,  0.0,  1.0,
     0.0,  0.0,  1.0,
     0.0,  0.0,  1.0,
     0.0,  0.0,  1.0,
     0.0,  0.0,  1.0,
     0.0,  0.0,  1.0,
    // top
     0.0,  1.0,  0.0,
     0.0,  1.0,  0.0,
     0.0,  1.0,  0.0,
     0.0,  1.0,  0.0,
     0.0,  1.0,  0.0,
     0.0,  1.0,  0.0,
    // back
     0.0,  0.0, -1.0,
     0.0,  0.0, -1.0,
     0.0,  0.0, -1.0,
     0.0,  0.0, -1.0,
     0.0,  0.0, -1.0,
     0.0,  0.0, -1.0,
    // bottom
     0.0, -1.0,  0.0,
     0.0, -1.0,  0.0,
     0.0, -1.0,  0.0,
     0.0, -1.0,  0.0,
     0.0, -1.0,  0.0,
     0.0, -1.0,  0.0,
    // left
    -1.0,  0.0,  0.0,
    -1.0,  0.0,  0.0,
    -1.0,  0.0,  0.0,
    -1.0,  0.0,  0.0,
    -1.0,  0.0,  0.0,
    -1.0,  0.0,  0.0,
    // right
     1.0,  0.0,  0.0,
     1.0,  0.0,  0.0,
     1.0,  0.0,  0.0,
     1.0,  0.0,  0.0,
     1.0,  0.0,  0.0,
     1.0,  0.0,  0.0,
};

static const float cube_t[] = {
    // front
    0.0, 0.0,
    1.0, 0.0,
    1.0, 1.0,
    1.0, 1.0,
    0.0, 1.0,
    0.0, 0.0,
    // top
    0.0, 0.0,
    1.0, 0.0,
    1.0, 1.0,
    1.0, 1.0,
    0.0, 1.0,
    0.0, 0.0,
    // back
    0.0, 0.0,
    1.0, 0.0,
    1.0, 1.0,
    1.0, 1.0,
    0.0, 1.0,
    0.0, 0.0,
    // bottom
    0.0, 0.0,
    1.0, 0.0,
    1.0, 1.0,
    1.0, 1.0,
    0.0, 1.0,
    0.0, 0.0,
    // left
    0.0, 0.0,
    1.0, 0.0,
    1.0, 1.0,
    1.0, 1.0,
    0.0, 1.0,
    0.0, 0.0,
    // right
    0.0, 0.0,
    1.0, 0.0,
    1.0, 1.0,
    1.0, 1.0,
    0.0, 1.0,
    0.0, 0.0,
};

enum {
    ATTRIB_POSITION,
    ATTRIB_NORMAL,
    ATTRIB_TEXCOORD,
};

enum {
    TEX_ALBEDO,
    TEX_NORMAL,
    TEX_REFRACTION,
    // TEX_GLOSS,
    TEX_EMISSION,
};

typedef struct ilG_computer {
    ilG_renderman *rm;
    ilG_matid mat;
    tgl_vao vao;
    GLuint v_vbo, n_vbo, t_vbo;
    GLuint mvp_loc, imt_loc;
    ilG_tex tex_albedo, tex_normal, tex_refraction, tex_emission;
} ilG_computer;

static void comp_draw(void *obj, ilG_rendid id, il_mat **mats, const unsigned *objects, unsigned num_mats)
{
    (void)id, (void)objects;
    ilG_computer *self = obj;
    ilG_material *mat = ilG_renderman_findMaterial(self->rm, self->mat);

    ilG_tex_bind(&self->tex_albedo);
    ilG_tex_bind(&self->tex_normal);
    ilG_tex_bind(&self->tex_refraction);
    ilG_tex_bind(&self->tex_emission);

    ilG_material_bind(mat);
    tgl_vao_bind(&self->vao);
    for (unsigned i = 0; i < num_mats; i++) {
        ilG_material_bindMatrix(mat, self->mvp_loc, mats[0][i]);
        ilG_material_bindMatrix(mat, self->imt_loc, mats[1][i]);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

static void comp_free(void *obj)
{
    ilG_computer *self = obj;
    ilG_renderman_delMaterial(self->rm, self->mat);
    tgl_vao_free(&self->vao);
    glDeleteBuffers(1, &self->v_vbo);
    glDeleteBuffers(1, &self->n_vbo);
    glDeleteBuffers(1, &self->t_vbo);
    free(self);
}

static bool comp_build(void *obj, ilG_rendid id, ilG_renderman *rm, ilG_buildresult *out)
{
    (void)id;
    ilG_computer *self = obj;
    self->rm = rm;

    ilG_material m;
    ilG_material_init(&m);
    ilG_material_name(&m, "Computer Shader");
    ilG_material_arrayAttrib(&m, ATTRIB_POSITION,     "in_Position");
    ilG_material_arrayAttrib(&m, ATTRIB_NORMAL,       "in_Normal");
    ilG_material_arrayAttrib(&m, ATTRIB_TEXCOORD,     "in_Texcoord");
    ilG_material_fragData(&m, ILG_CONTEXT_ALBEDO,     "out_Albedo");
    ilG_material_fragData(&m, ILG_CONTEXT_NORMAL,     "out_Normal");
    ilG_material_fragData(&m, ILG_CONTEXT_REFRACTION, "out_Refraction");
    ilG_material_fragData(&m, ILG_CONTEXT_GLOSS,      "out_Gloss");
    ilG_material_fragData(&m, ILG_CONTEXT_EMISSION,   "out_Emission");
    ilG_material_textureUnit(&m, TEX_ALBEDO,          "tex_Albedo");
    ilG_material_textureUnit(&m, TEX_NORMAL,          "tex_Normal");
    ilG_material_textureUnit(&m, TEX_REFRACTION,      "tex_Reflect");
    ilG_material_textureUnit(&m, TEX_EMISSION,        "tex_Emission");
    if (!ilG_renderman_addMaterialFromFile(rm, m, "comp.vert", "comp.frag", &self->mat, &out->error)) {
        return false;
    }
    ilG_material *mat = ilG_renderman_findMaterial(rm, self->mat);
    self->mvp_loc = ilG_material_getLoc(mat, "mvp");
    self->imt_loc = ilG_material_getLoc(mat, "imt");

    tgl_vao_init(&self->vao);
    tgl_vao_bind(&self->vao);
    glGenBuffers(1, &self->v_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, self->v_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
    glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(ATTRIB_POSITION);
    glGenBuffers(1, &self->n_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, self->n_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_n), cube_n, GL_STATIC_DRAW);
    glVertexAttribPointer(ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(ATTRIB_NORMAL);
    glGenBuffers(1, &self->t_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, self->t_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_t), cube_t, GL_STATIC_DRAW);
    glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(ATTRIB_TEXCOORD);

    ilG_tex *texes[4] = {
        &self->tex_albedo,
        &self->tex_normal,
        &self->tex_refraction,
        &self->tex_emission
    };
    static const char *const files[] = {
        "comp1a.png",
        "comp1_n.png",
        "comp1_s.png",
        "comp1a_g.png"
    };
    for (unsigned i = 0; i < 4; i++) {
        ilA_imgerr err;
        extern ilA_fs demo_fs;
        if ((err = ilG_tex_loadfile(texes[i], &demo_fs, files[i]))) {
            il_error("%s", ilA_img_strerror(err));
            return false;
        }
        ilG_tex_build(texes[i]);
        texes[i]->unit = i;
    }

    int *types = calloc(2, sizeof(int));
    types[0] = ILG_MVP;
    types[1] = ILG_IMT;
    out->free = comp_free;
    out->draw = comp_draw;
    out->types = types;
    out->num_types = 2;
    out->obj = obj;
    out->name = strdup("Computer");
    return true;
}

ilG_builder ilG_computer_builder()
{
    ilG_computer *self = calloc(1, sizeof(ilG_computer));
    return ilG_builder_wrap(self, comp_build);
}
