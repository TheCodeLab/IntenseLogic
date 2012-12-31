#include "shape.h"

#include <stdlib.h>

#include <GL/glew.h>

#include "common/log.h"
#include "common/string.h"
#include "asset/asset.h"
#include "graphics/glutil.h"
#include "graphics/drawable3d.h"
#include "graphics/tracker.h"
#include "graphics/context.h"

static float cube[] = {
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

static float cube_texcoord[] = {
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


static short cube_index[] = {
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

struct ilG_shape {
    ilG_drawable3d drawable;
    GLuint vbo;
    GLuint ibo;
    GLuint vao;
    GLenum mode;
    GLsizei count;
};

static void draw(ilG_context* context, il_positionable* pos, void * ctx)
{
    (void)ctx;
    (void)pos;
    struct ilG_shape * shape = (struct ilG_shape*)context->drawable;

    glBindVertexArray(shape->vao);
    IL_GRAPHICS_TESTERROR("Could not bind vao");

    glBindBuffer(GL_ARRAY_BUFFER, shape->vbo);
    IL_GRAPHICS_TESTERROR("Could not bind vbo");

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape->ibo);
    IL_GRAPHICS_TESTERROR("Could not bind ibo");

    glDrawElements(shape->mode, shape->count, GL_UNSIGNED_SHORT, (GLvoid*)0);
    IL_GRAPHICS_TESTERROR("Could not draw ibo");
}

static struct ilG_shape box, cylinder, sphere, plane;

void ilG_shape_init()
{
    GLuint vao[4], vbo[4], ibo[4];
    int i;
    
    memset(&box,        0, sizeof(struct ilG_shape));
    memset(&cylinder,   0, sizeof(struct ilG_shape));
    memset(&sphere,     0, sizeof(struct ilG_shape));
    memset(&plane,      0, sizeof(struct ilG_shape));

    box.drawable.name       = "Box Primitive";
    cylinder.drawable.name  = "Cylinder Primitive";
    sphere.drawable.name    = "Sphere Primitive";
    plane.drawable.name     = "Plane Primitive";

    // assignment is an expression that returns what was assigned
    box.drawable.draw       =
    cylinder.drawable.draw  =
    sphere.drawable.draw    = 
    plane.drawable.draw     = &draw;

    // don't want the wrong error to pop up, has happened before
    IL_GRAPHICS_TESTERROR("Unknown error before this function");

    glGenVertexArrays(4, &vao[0]);
    IL_GRAPHICS_TESTERROR("Unable to generate vertex array");
    box.vao         = vao[0];
    cylinder.vao    = vao[1];
    sphere.vao      = vao[2];
    plane.vao       = vao[3];

    glGenBuffers(4, &vbo[0]);
    IL_GRAPHICS_TESTERROR("Unable to generate vertex buffer");
    box.vbo         = vbo[0];
    cylinder.vbo    = vbo[1];
    sphere.vbo      = vbo[2];
    plane.vbo       = vbo[3];

    glGenBuffers(4, &ibo[0]);
    IL_GRAPHICS_TESTERROR("Unable to generate index buffer object");
    box.ibo         = ibo[0];
    cylinder.ibo    = ibo[1];
    sphere.ibo      = ibo[2];
    plane.ibo       = ibo[3];

    for (i = 0; i < 4; i++) {
        glBindVertexArray(vao[i]);
        IL_GRAPHICS_TESTERROR("Unable to bind vertex array");
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);
        IL_GRAPHICS_TESTERROR("Unable to bind vertex buffer");

        // store vertices and texcoords in the same buffer
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube) + sizeof(cube_texcoord), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0,             sizeof(cube), cube);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(cube),  sizeof(cube_texcoord), cube_texcoord);
        IL_GRAPHICS_TESTERROR("Unable to upload cube data");

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)sizeof(cube));
        IL_GRAPHICS_TESTERROR("Unable to set vertex attrib pointer");

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        IL_GRAPHICS_TESTERROR("Unable to enable vertex attrib array index 0");

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[i]);
        IL_GRAPHICS_TESTERROR("Unable to bind index buffer object");

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_index), cube_index, GL_STATIC_DRAW);
        IL_GRAPHICS_TESTERROR("Unable to upload index buffer data");

        box.mode = GL_TRIANGLES;
        box.count = 36;
    }

    ilG_box         = &box.drawable;
    ilG_cylinder    = &cylinder.drawable;
    ilG_sphere      = &sphere.drawable;
    ilG_plane       = &plane.drawable;

    ilG_drawable3d_assignId(ilG_box);
    ilG_drawable3d_assignId(ilG_cylinder);
    ilG_drawable3d_assignId(ilG_sphere);
    ilG_drawable3d_assignId(ilG_plane);
}

