#include "quad.h"

#include <GL/glew.h>

#include "graphics/context.h"
#include "graphics/arrayattrib.h"
#include "graphics/bindable.h"

struct ilG_quad {
    GLuint vbo, vao;
    int valid;
};

static void quad_free(void *ptr)
{
    ilG_quad *self = ptr;
    glDeleteBuffers(1, &self->vbo);
    glDeleteBuffers(1, &self->vao);
    free(self);
}

ilG_quad *ilG_quad_get(ilG_context* context)
{
    static const float verts[] = {
        0.f, 0.f,
        1.f, 0.f,
        1.f, 1.f,
        0.f, 1.f,
    };
    ilG_quad *q = il_table_mgetsp(&context->storage, "quad");
    if (q) {
        return q;
    }
    q = calloc(1, sizeof(ilG_quad));
    glGenBuffers(1, &q->vbo);
    glGenVertexArrays(1, &q->vao);
    glBindBuffer(GL_ARRAY_BUFFER, q->vbo);
    glBindVertexArray(q->vao);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(ILG_ARRATTR_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glVertexAttribPointer(ILG_ARRATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(ILG_ARRATTR_POSITION);
    glEnableVertexAttribArray(ILG_ARRATTR_TEXCOORD);
    il_table_setsp(&context->storage, "quad", il_opaque(q, quad_free));
    return q;
}

void ilG_quad_bind(ilG_quad *q)
{
    glBindBuffer(GL_ARRAY_BUFFER, q->vbo);
    glBindVertexArray(q->vao);
}

void ilG_quad_draw(ilG_quad *q)
{
    (void)q;
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}
