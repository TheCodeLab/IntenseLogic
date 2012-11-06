#include "shape.h"

#include <stdlib.h>

#include <GL/glew.h>

#include "common/log.h"
#include "common/string.h"
#include "asset/asset.h"
#include "graphics/glutil.h"

/*
o Cube
v 1.000000 -1.000000 -1.000000
v 1.000000 -1.000000 1.000000
v -1.000000 -1.000000 1.000000
v -1.000000 -1.000000 -1.000000
v 1.000000 1.000000 -0.999999
v 0.999999 1.000000 1.000001
v -1.000000 1.000000 1.000000
v -1.000000 1.000000 -1.000000
f 1 2 3 4
f 5 8 7 6
f 1 5 6 2
f 2 6 7 3
f 3 7 8 4
f 5 1 4 8
*/

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

struct il_Graphics_Shape {
  il_Graphics_Drawable3d drawable;
  int type;
  GLuint vbo;
  GLuint ibo;
  GLuint vao;
  GLuint program;
  GLuint texture;
  GLenum mode;
  GLsizei count;
};

static void shape_draw(const il_Graphics_Camera* cam, struct il_Graphics_Drawable3d* drawable, const struct timeval* tv) {
  (void)tv;
  il_Graphics_Shape * shape = (il_Graphics_Shape*)drawable;
  
  glUseProgram(shape->program);
  IL_GRAPHICS_TESTERROR("Could not use program");
  
  il_Graphics_bindUniforms(shape->program, cam, drawable->positionable);
  
  glBindBuffer(GL_ARRAY_BUFFER, shape->vbo);
  IL_GRAPHICS_TESTERROR("Could not bind vbo");
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape->ibo);
  IL_GRAPHICS_TESTERROR("Could not bind ibo");
  
  //glActiveTexture(GL_TEXTURE0);
  //glBindTexture(GL_TEXTURE_2D, shape->texture);
  
  //glEnable(GL_PRIMITIVE_RESTART);
  
  //glPrimitiveRestartIndex((unsigned short)-1);
  
  glDrawElements(shape->mode, shape->count, GL_UNSIGNED_SHORT, (GLvoid*)0);
  IL_GRAPHICS_TESTERROR("Could not draw ibo");
}

il_Graphics_Shape * il_Graphics_Shape_new(il_Common_Positionable * parent, int type) {
  il_Graphics_Shape * shape = calloc(1, sizeof(il_Graphics_Shape));
  shape->type = type;
  shape->drawable.type = ('P'<<24) + ('R'<<16) + ('I'<<8) + ('M'<<0);
  //shape->drawable.positionable = parent;
  shape->drawable.draw = (il_Graphics_Drawable3d_cb)&shape_draw;
  
  glGenVertexArrays(1, &shape->vao);
  IL_GRAPHICS_TESTERROR("Unable to generate vertex array");
  glBindVertexArray(shape->vao);
  IL_GRAPHICS_TESTERROR("Unable to bind vertex array");
  
  glGenBuffers(1, &shape->vbo);
  IL_GRAPHICS_TESTERROR("Unable to generate vertex buffer");
  glBindBuffer(GL_ARRAY_BUFFER, shape->vbo);
  IL_GRAPHICS_TESTERROR("Unable to bind vertex buffer");
  switch(type) {
    case il_Graphics_Box: {
      glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
      IL_GRAPHICS_TESTERROR("Unable to upload cube data");
      break;
    }
  }
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  IL_GRAPHICS_TESTERROR("Unable to set vertex attrib pointer");
  glEnableVertexAttribArray(0);
  IL_GRAPHICS_TESTERROR("Unable to enable vertex attrib array index 0");
  
  glGenBuffers(1, &shape->ibo);
  IL_GRAPHICS_TESTERROR("Unable to generate index buffer object");
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape->ibo);
  IL_GRAPHICS_TESTERROR("Unable to bind index buffer object");
  switch(type) {
    case il_Graphics_Box: {
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_index), cube_index, GL_STATIC_DRAW);
      IL_GRAPHICS_TESTERROR("Unable to upload index buffer data");
    }
  }
  
  shape->mode = GL_TRIANGLES;
  shape->count = 36;
  
  shape->program = glCreateProgram();
  IL_GRAPHICS_TESTERROR("Unable to create program");
  
  GLuint vertex, fragment;
  il_Common_String vertex_source, fragment_source;
  
  vertex_source = IL_ASSET_READFILE("cube.vert");
  fragment_source = IL_ASSET_READFILE("cube.frag");
  
  if (!vertex_source.length) {
    il_Common_log(1, "Unable to open cube vertex shader");
    return NULL;
  }
  if (!fragment_source.length) {
    il_Common_log(1, "Unable to open cube fragment shader");
    return NULL;
  }
  
  vertex = il_Graphics_makeShader(GL_VERTEX_SHADER, vertex_source);
  fragment = il_Graphics_makeShader(GL_FRAGMENT_SHADER, fragment_source);
  
  glAttachShader(shape->program, vertex);
  IL_GRAPHICS_TESTERROR("Unable to attach shader");
  glAttachShader(shape->program, fragment);
  IL_GRAPHICS_TESTERROR("Unable to attach shader");
  
  il_Graphics_linkProgram(shape->program);
  
  il_Graphics_Drawable3d_setPositionable(&shape->drawable, parent);
  
  return shape;
}
