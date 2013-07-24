#include "heightmap.h"

#include <GL/glew.h>

#include "graphics/context.h"
#include "graphics/drawable3d.h"
#include "graphics/material.h"
#include "graphics/arrayattrib.h"
#include "graphics/bindable.h"
#include "graphics/textureunit.h"

struct heightmap {
    ilG_drawable3d drawable;
    GLuint vbo, vao;
    int count;
};

il_type ilG_heightmap_type = {
    .name = "il.graphics.heightmap",
    .size = sizeof(struct heightmap),
    .parent = &ilG_drawable3d_type
};

static void heightmap_bind(void *obj)
{
    struct heightmap *self = obj;
    glBindVertexArray(self->vao);
    glBindBuffer(GL_ARRAY_BUFFER, self->vbo);
}

static void heightmap_draw(void *obj)
{
    struct heightmap *self = obj;
    glDrawArrays(GL_TRIANGLES, 0, self->count);
}

static ilG_bindable heightmap_bindable = {
    .name = "il.graphics.bindable",
    .bind = heightmap_bind,
    .action = heightmap_draw
};

void ilG_heightmap_init()
{
    il_impl(&ilG_heightmap_type, &heightmap_bindable);
}

ilG_drawable3d *ilG_heightmap_new(ilG_context *context, unsigned w, unsigned h)
{
    (void) context;

    float *buf = calloc(w * h, sizeof(float) * 6 * 2);
    unsigned x, y;
    
    for (y = 1; y < h; y++) {
        for (x = 1; x < w; x++) {
            float patch[] = {
                x,   y,
                x-1, y,
                x,   y-1,

                x,   y-1,
                x-1, y,
                x-1, y-1,
            };
            memcpy(buf + y * w * 12 + x * 12, patch, sizeof(patch));
        }
    }
    struct heightmap *hm = il_new(&ilG_heightmap_type);
    hm->count = w * h * 6;
    glGenVertexArrays(1, &hm->vao);
    glBindVertexArray(hm->vao);
    glGenBuffers(1, &hm->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, hm->vbo);
    glBufferData(GL_ARRAY_BUFFER, w * h * 6 * 2 * sizeof(float), buf, GL_STATIC_DRAW);
    free(buf);
    glVertexAttribPointer(ILG_ARRATTR_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(ILG_ARRATTR_POSITION);
    ILG_SETATTR(hm->drawable.attrs, ILG_ARRATTR_POSITION);
    return &hm->drawable;
}

ilG_material *ilG_heightmap_shader(ilG_context *context)
{
    ilG_material *mat = il_base_get(context, "il.graphics.heightmap.shader", NULL, NULL);
    if (mat) {
        return mat;
    }
    mat = ilG_material_new();
    ilG_material_vertex_file(mat, "heightmap.vert");
    ilG_material_fragment_file(mat, "heightmap.frag");
    ilG_material_name(mat, "Heightmap Shader");
    ilG_material_arrayAttrib(mat, ILG_ARRATTR_POSITION, "in_Position");
    ilG_material_textureUnit(mat, ILG_TUNIT_HEIGHT0, "height_tex");
    ilG_material_matrix(mat, ILG_MVP, "mvp");
    if (!ilG_material_link(mat, context)) {
        il_unref(mat);
        return NULL;
    }
    il_base_set(context, "il.graphics.heightmap.shader", mat, 0, IL_OBJECT);

    return mat;
}

