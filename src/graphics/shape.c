#include "shape.h"

#include <stdlib.h>

#include <GL/glew.h>

#include "asset/asset.h"
#include "graphics/glutil.h"
#include "graphics/drawable3d.h"
#include "graphics/bindable.h"
#include "graphics/tracker.h"
#include "graphics/context.h"
#include "graphics/arrayattrib.h"

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
    0.0,    0.5,
    0.3333, 0.5,
    0.3333, 1.0,
    0.0,    1.0,
    // top
    0.3333, 0.5,
    0.6666, 0.5,
    0.6666, 1.0,
    0.3333, 1.0,
    // back
    0.0,    0.0,
    0.3333, 0.0,
    0.3333, 0.5,
    0.0,    0.5,
    // bottom
    0.3333, 0.0,
    0.6666, 0.0,
    0.6666, 0.5,
    0.3333, 0.5,
    // left
    0.6666, 0.5,
    1.0,    0.5,
    1.0,    1.0,
    0.6666, 1.0,
    // right
    0.6666, 0.0,
    1.0,    0.0,
    1.0,    0.5,
    0.6666, 0.5,
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
static short ico_index[] = {
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

struct ilG_shape {
    ilG_drawable3d drawable;
    GLuint vbo;
    GLuint ibo;
    GLuint vao;
    GLenum mode;
    GLsizei count;
};

static void bind(void *obj)
{
    struct ilG_shape * shape = obj;
    glBindVertexArray(shape->vao);
    glBindBuffer(GL_ARRAY_BUFFER, shape->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape->ibo);
    IL_GRAPHICS_TESTERROR("Could not bind drawable");
}

static void draw(void *obj)
{
    struct ilG_shape * shape = obj;

    glDrawElements(shape->mode, shape->count, GL_UNSIGNED_SHORT, (GLvoid*)0);
    IL_GRAPHICS_TESTERROR("Could not draw drawable");
}

static struct ilG_shape box, cylinder, icosahedron, plane;

il_type ilG_shape_type = {
    .typeclasses = NULL,
    .storage = NULL,
    .constructor = NULL,
    .name = "il.graphics.shape",
    .registry = NULL,
    .size = sizeof(struct ilG_shape),
    .parent = &ilG_drawable3d_type
};

static ilG_bindable shape_bindable = {
    .name = "il.graphics.bindable",
    .hh = {0},
    .bind = &bind,
    .action = &draw
};

void ilG_shape_init()
{
    GLuint vao[4], vbo[4], ibo[4];
    int i;

    il_impl(&ilG_shape_type, &shape_bindable);
    
    memset(&box,        0, sizeof(struct ilG_shape));
    memset(&cylinder,   0, sizeof(struct ilG_shape));
    memset(&icosahedron,0, sizeof(struct ilG_shape));
    memset(&plane,      0, sizeof(struct ilG_shape));

    il_init(&ilG_shape_type, &box);
    il_init(&ilG_shape_type, &cylinder);
    il_init(&ilG_shape_type, &icosahedron);
    il_init(&ilG_shape_type, &plane);

    // don't want the wrong error to pop up, has happened before
    IL_GRAPHICS_TESTERROR("Unknown error before this function");

    glGenVertexArrays(4, &vao[0]);
    IL_GRAPHICS_TESTERROR("Unable to generate vertex array");
    box.vao         = vao[0];
    cylinder.vao    = vao[1];
    icosahedron.vao = vao[2];
    plane.vao       = vao[3];

    glGenBuffers(4, &vbo[0]);
    IL_GRAPHICS_TESTERROR("Unable to generate vertex buffer");
    box.vbo         = vbo[0];
    cylinder.vbo    = vbo[1];
    icosahedron.vbo = vbo[2];
    plane.vbo       = vbo[3];

    glGenBuffers(4, &ibo[0]);
    IL_GRAPHICS_TESTERROR("Unable to generate index buffer object");
    box.ibo         = ibo[0];
    cylinder.ibo    = ibo[1];
    icosahedron.ibo = ibo[2];
    plane.ibo       = ibo[3];

    ILG_SETATTR(box.drawable.attrs, ILG_ARRATTR_POSITION);
    ILG_SETATTR(box.drawable.attrs, ILG_ARRATTR_TEXCOORD);

    ILG_SETATTR(icosahedron.drawable.attrs, ILG_ARRATTR_POSITION);

    for (i = 0; i < 4; i++) {
        glBindVertexArray(vao[i]);
        IL_GRAPHICS_TESTERROR("Unable to bind vertex array");
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);
        IL_GRAPHICS_TESTERROR("Unable to bind vertex buffer");

        // store vertices and texcoords in the same buffer
        switch(i) {
          case 0:
            glBufferData(GL_ARRAY_BUFFER, sizeof(cube) + sizeof(cube_texcoord), NULL, GL_STATIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0,             sizeof(cube), cube);
            glBufferSubData(GL_ARRAY_BUFFER, sizeof(cube),  sizeof(cube_texcoord), cube_texcoord);
            IL_GRAPHICS_TESTERROR("Unable to upload cube data");
            glVertexAttribPointer(ILG_ARRATTR_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glVertexAttribPointer(ILG_ARRATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)sizeof(cube));
            IL_GRAPHICS_TESTERROR("Unable to set vertex attrib pointer");
            break;
          case 2:
            glBufferData(GL_ARRAY_BUFFER, sizeof(ico) /*+ sizeof(ico_texcoord)*/, NULL, GL_STATIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0,             sizeof(ico), ico);
            //glBufferSubData(GL_ARRAY_BUFFER, sizeof(ico),   sizeof(cube_texcoord), cube_texcoord);
            IL_GRAPHICS_TESTERROR("Unable to upload icosahedron data");
            glVertexAttribPointer(ILG_ARRATTR_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            //glVertexAttribPointer(ILG_ARRATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)sizeof(cube));
            IL_GRAPHICS_TESTERROR("Unable to set vertex attrib pointer");
            break;
          default:
            break;
        }

        glEnableVertexAttribArray(ILG_ARRATTR_POSITION);
        glEnableVertexAttribArray(ILG_ARRATTR_TEXCOORD);
        IL_GRAPHICS_TESTERROR("Unable to enable vertex attrib array index 0");

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[i]);
        IL_GRAPHICS_TESTERROR("Unable to bind index buffer object");

        switch(i) {
          case 0:
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_index), cube_index, GL_STATIC_DRAW);
            IL_GRAPHICS_TESTERROR("Unable to upload index buffer data");
            box.mode = GL_TRIANGLES;
            box.count = 36;
            break;
          case 2:
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ico_index), ico_index, GL_STATIC_DRAW);
            IL_GRAPHICS_TESTERROR("Unable to upload index buffer data");
            icosahedron.mode = GL_TRIANGLES;
            icosahedron.count = 20 * 3;
            break;
          default:
            break;
        }
    }

    ilG_box         = &box.drawable;
    ilG_cylinder    = &cylinder.drawable;
    ilG_icosahedron = &icosahedron.drawable;
    ilG_plane       = &plane.drawable;

    ilG_drawable3d_assignId(ilG_box);
    ilG_drawable3d_assignId(ilG_cylinder);
    ilG_drawable3d_assignId(ilG_icosahedron);
    ilG_drawable3d_assignId(ilG_plane);
}

