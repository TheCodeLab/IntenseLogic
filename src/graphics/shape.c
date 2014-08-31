#include "shape.h"

#include <stdlib.h>

#include "tgl/tgl.h"
#include "graphics/arrayattrib.h"
#include "graphics/context.h"

struct ilG_shape {
    ilG_handle handle;
    GLuint vbo;
    GLuint ibo;
    GLuint vao;
    GLenum mode;
    GLsizei count;
    int type;
};

static const float cube[] = {
    // front
    -1.0, -1.0,  1.0,
    1.0, -1.0,  1.0,
    1.0,  1.0,  1.0,
    -1.0,  1.0,  1.0,
    // top
    -1.0,  1.0,  1.0,
    1.0,  1.0,  1.0,
    1.0,  1.0, -1.0,
    -1.0,  1.0, -1.0,
    // back
    1.0, -1.0, -1.0,
    -1.0, -1.0, -1.0,
    -1.0,  1.0, -1.0,
    1.0,  1.0, -1.0,
    // bottom
    -1.0, -1.0, -1.0,
    1.0, -1.0, -1.0,
    1.0, -1.0,  1.0,
    -1.0, -1.0,  1.0,
    // left
    -1.0, -1.0, -1.0,
    -1.0, -1.0,  1.0,
    -1.0,  1.0,  1.0,
    -1.0,  1.0, -1.0,
    // right
    1.0, -1.0,  1.0,
    1.0, -1.0, -1.0,
    1.0,  1.0, -1.0,
    1.0,  1.0,  1.0,
};

static const float cube_texcoord[] = {
    // front
    -1.0,  1.0,  1.0,
     1.0,  1.0,  1.0,
     1.0, -1.0,  1.0,
    -1.0, -1.0,  1.0,
    // top
     1.0,  1.0,  1.0,
     1.0,  1.0, -1.0,
    -1.0,  1.0, -1.0,
    -1.0,  1.0,  1.0,
    // back
     1.0,  1.0, -1.0,
    -1.0,  1.0, -1.0,
    -1.0, -1.0, -1.0,
     1.0, -1.0, -1.0,
    // bottom
     1.0, -1.0,  1.0,
     1.0, -1.0, -1.0,
    -1.0, -1.0, -1.0,
    -1.0, -1.0,  1.0,
    // left
    -1.0,  1.0, -1.0,
    -1.0,  1.0,  1.0,
    -1.0, -1.0,  1.0,
    -1.0, -1.0, -1.0,
    // right
     1.0,  1.0,  1.0,
     1.0,  1.0, -1.0,
     1.0, -1.0, -1.0,
     1.0, -1.0,  1.0,
};

static const short cube_index[] = {
    // front
    0,  1,  2,
    2,  3,  0,
    // top
    4,  5,  6,
    6,  7,  4,
    // back
    8,  9, 10,
    10, 11,  8,
    // bottom
    12, 13, 14,
    14, 15, 12,
    // left
    16, 17, 18,
    18, 19, 16,
    // right
    20, 21, 22,
    22, 23, 20,
};

// icosahedron code lifted from http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html
#define ico_t 1.618033988749895
//(1.0 + sqrt(5.0)) / 2.0; // no compile time sqrt() :( // or using consts in other consts :(
static const GLfloat ico[] = {
    -1,  ico_t,  0,
     1,  ico_t,  0,
    -1, -ico_t,  0,
     1, -ico_t,  0,

     0, -1,  ico_t,
     0,  1,  ico_t,
     0, -1, -ico_t,
     0,  1, -ico_t,

     ico_t,  0, -1,
     ico_t,  0,  1,
    -ico_t,  0, -1,
    -ico_t,  0,  1,
};
#undef ico_t
static const short ico_index[] = {
    0, 11, 5,
    0, 5, 1,
    0, 1, 7,
    0, 7, 10,
    0, 10, 11,

    1, 5, 9,
    5, 11, 4,
    11, 10, 2,
    10, 7, 6,
    7, 1, 8,

    3, 9, 4,
    3, 4, 2,
    3, 2, 6,
    3, 6, 8,
    3, 8, 9,

    4, 9, 5,
    2, 4, 11,
    6, 2, 10,
    8, 6, 7,
    9, 8, 1,
};

ilG_shape *ilG_box(ilG_context *context)
{
    struct ilG_shape *self = il_table_mgetsp(&context->storage, "shape.box");
    if (self) {
        return self;
    }
    self = calloc(1, sizeof(ilG_shape));
    self->mode = GL_TRIANGLES;
    self->count = 36;
    self->type = 1;

    glGenVertexArrays(1, &self->vao);
    glGenBuffers(1, &self->vbo);
    glGenBuffers(1, &self->ibo);

    glBindVertexArray(self->vao);
    glBindBuffer(GL_ARRAY_BUFFER, self->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->ibo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(cube) + sizeof(cube_texcoord), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,             sizeof(cube), cube);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(cube),  sizeof(cube_texcoord), cube_texcoord);
    tgl_check("Unable to upload cube data");
    glVertexAttribPointer(ILG_ARRATTR_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glVertexAttribPointer(ILG_ARRATTR_TEXCOORD, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)sizeof(cube));
    glEnableVertexAttribArray(ILG_ARRATTR_POSITION);
    glEnableVertexAttribArray(ILG_ARRATTR_TEXCOORD);
    tgl_check("Unable to set vertex attrib pointer");
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_index), cube_index, GL_STATIC_DRAW);
    tgl_check("Unable to upload index buffer data");
    il_table_setsp(&context->storage, "shape.box", il_opaque(self, free));
    return self;
}

ilG_shape *ilG_icosahedron(ilG_context *context)
{
    struct ilG_shape *self = il_table_mgetsp(&context->storage, "shape.icosahedron");
    if (self) {
        return self;
    }
    self = calloc(1, sizeof(ilG_shape));
    self->mode = GL_TRIANGLES;
    self->count = 20 * 3;
    self->type = 2;
    glGenVertexArrays(1, &self->vao);
    glGenBuffers(1, &self->vbo);
    glGenBuffers(1, &self->ibo);

    glBindVertexArray(self->vao);
    glBindBuffer(GL_ARRAY_BUFFER, self->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->ibo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(ico) /*+ sizeof(ico_texcoord)*/, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,             sizeof(ico), ico);
    //glBufferSubData(GL_ARRAY_BUFFER, sizeof(ico),   sizeof(cube_texcoord), cube_texcoord);
    tgl_check("Unable to upload icosahedron data");
    glVertexAttribPointer(ILG_ARRATTR_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(ILG_ARRATTR_POSITION);
    //glVertexAttribPointer(ILG_ARRATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)sizeof(cube));
    tgl_check("Unable to set vertex attrib pointer");
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ico_index), ico_index, GL_STATIC_DRAW);
    tgl_check("Unable to upload index buffer data");

    il_table_setsp(&context->storage, "shape.icosahedron", il_opaque(self, free));
    return self;
}

void ilG_shape_bind(ilG_shape *shape)
{
    glBindVertexArray(shape->vao);
    glBindBuffer(GL_ARRAY_BUFFER, shape->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape->ibo);
}

void ilG_shape_draw(ilG_shape *shape)
{
    tgl_check("Unknown");
    glDrawElements(shape->mode, shape->count, GL_UNSIGNED_SHORT, (GLvoid*)0);
    tgl_check("Could not draw shape");
}
