#include "quad.h"

#include <GL/glew.h>

#include "graphics/drawable3d.h"
#include "graphics/context.h"
#include "graphics/arrayattrib.h"
#include "graphics/bindable.h"

struct quad {
    ilG_drawable3d drawable;
    GLuint vbo, vao;
    int valid;
};

il_type ilG_quad_type = {
    .typeclasses = NULL,
    .storage = NULL,
    .constructor = NULL,
    .destructor = NULL,
    .copy = NULL,
    .name = "il.graphics.gui.quad",
    .size = sizeof(struct quad),
    .parent = &ilG_drawable3d_type
};

static void quad_bind(void *obj)
{
    struct quad *q = obj;
    glBindBuffer(GL_ARRAY_BUFFER, q->vbo);
    glBindVertexArray(q->vao);
}

static void quad_draw(void *obj)
{
    (void)obj;
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

static ilG_bindable quad_bindable = {
    .name = "il.graphics.bindable",
    .bind = quad_bind,
    .action = quad_draw
};

void ilG_quad_init()
{
    il_impl(&ilG_quad_type, &quad_bindable);
}

ilG_drawable3d *ilG_quad(ilG_context* context)
{
    static float verts[] = {
        0.f, 0.f,
        1.f, 0.f,
        1.f, 1.f,
        0.f, 1.f,
    };
    struct quad *q;
    if ((q = il_base_get(&context->base, "il.graphics.gui.quad", NULL, NULL))) {
        return &q->drawable;
    }
    q = il_new(&ilG_quad_type);
    q->drawable.context = context;
    q->drawable.name = "Fullscreen Quad";
    ILG_SETATTR(q->drawable.attrs, ILG_ARRATTR_POSITION);
    glGenBuffers(1, &q->vbo);
    glGenVertexArrays(1, &q->vao);
    glBindBuffer(GL_ARRAY_BUFFER, q->vbo);
    glBindVertexArray(q->vao);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(ILG_ARRATTR_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glVertexAttribPointer(ILG_ARRATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(ILG_ARRATTR_POSITION);
    glEnableVertexAttribArray(ILG_ARRATTR_TEXCOORD);
    q->valid = 1;
    il_base_set(&context->base, "il.graphics.gui.quad", q, 0, IL_OBJECT|IL_LOCAL_BIT);
    return &q->drawable;
}

