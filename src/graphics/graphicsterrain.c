#include "terrain.h"

#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <math.h>

#include "graphics/drawable3d.h"
#include "asset/asset.h"
#include "common/string.h"
#include "common/log.h"
#include "graphics/glutil.h"

enum terrain_type {
  NONE = 0,
  HEIGHTMAP,
  PHEIGHTMAP,
};

struct il_Common_Terrain {
  int type;
  int width, height;
  size_t size;
  void * data;
  int refs;
  void *destruct_ctx;
  void (*destruct)(il_Common_Terrain*, void*);
  void *point_ctx;
  double (*getPoint)(il_Common_Terrain*, void*, unsigned x, unsigned y, 
    double height);
  void *normal_ctx;
  sg_Vector3 (*getNormal)(il_Common_Terrain*, void*, unsigned x, unsigned y, 
    double z);
};

struct il_Graphics_Terrain {
  il_Graphics_Drawable3d drawable;
  il_Common_Terrain * terrain;
  GLuint *buf;
  GLuint program;
  void *draw_ctx;
  void (*draw)(il_Graphics_Terrain*, void*, const il_Graphics_Camera*, 
    const struct timeval*);
};

struct pheightmap {
  long long seed;
  float resolution;
  float viewdistance;
};

static void terrain_draw(const il_Graphics_Camera* cam, 
  struct il_Graphics_Drawable3d* drawable, const struct timeval* tv) 
{
  il_Graphics_Terrain* ter = (il_Graphics_Terrain*)drawable;
  ter->draw(ter, ter->draw_ctx, cam, tv);
}

static void heightmap_draw(il_Graphics_Terrain* ter, void* ctx, 
  const il_Graphics_Camera* cam, const struct timeval* tv) 
{
  (void)ctx, (void)cam, (void)tv;
  glUseProgram(ter->program);
  
  //activate the heightmap texture
  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D, ter->buf[0]);
  
  glDrawArrays(GL_TRIANGLE_FAN, 0, ter->terrain->width * ter->terrain->height);
}

static void pheightmap_draw(il_Graphics_Terrain* ter, void* ctx,
  const il_Graphics_Camera* cam, const struct timeval* tv)
{
  (void)tv;
  struct pheightmap* pheightmap = ctx;
  
  glUseProgram(ter->program);
  il_Graphics_testError("glUseProgram");
  
  il_Graphics_bindUniforms(ter->program, cam, ter->drawable.positionable);
  il_Graphics_testError("il_Graphics_bindUniforms");
  
  GLint center = glGetUniformLocation(ter->program, "center");
  il_Graphics_testError("glGetUniformLocation");
  glUniform2i(center, 
    (int)floor(ter->drawable.positionable->position.x / pheightmap->resolution),
    (int)floor(ter->drawable.positionable->position.z / pheightmap->resolution)
  );
  il_Graphics_testError("glUniform2i");
  
  glBindBuffer(GL_ARRAY_BUFFER, ter->buf[0]);
  il_Graphics_testError("glBindBuffer");
  glBindVertexArray(ter->buf[2]);
  il_Graphics_testError("glBindVertexArray");
  
  int width = pheightmap->viewdistance / pheightmap->resolution,
      size = width * width;
  
  glPrimitiveRestartIndex(-1);
  il_Graphics_testError("glPrimitiveRestartIndex");
  
  glEnable(GL_PRIMITIVE_RESTART);
  il_Graphics_testError("glEnable");
  
  glEnableVertexAttribArray(0);
  il_Graphics_testError("glEnableVertexAttribArray");
  
  glDrawArrays(GL_TRIANGLE_STRIP, 0, size*2);
  il_Graphics_testError("glDrawArrays");
  
  glDisableVertexAttribArray(0);
  il_Graphics_testError("glDisableVertexAttribArray");
  
  glDisable(GL_PRIMITIVE_RESTART);
  il_Graphics_testError("glDisable");
}

il_Graphics_Terrain* il_Graphics_Terrain_new(il_Common_Terrain* parent, 
  il_Common_Positionable* positionable) 
{
  il_Graphics_Terrain * ter = calloc(sizeof(il_Graphics_Terrain),1);
  ter->terrain = parent;
  ter->drawable.draw = (il_Graphics_Drawable3d_cb)&terrain_draw;
  switch(parent->type) {
    case HEIGHTMAP: {
      ter->buf = calloc(1, sizeof(GLuint));
      glGenTextures(1, ter->buf);
      glBindTexture(GL_TEXTURE_2D, ter->buf[0]);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, parent->width, parent->height, 0, 
        GL_RED, GL_FLOAT, parent->data);
      
      GLuint  vertex = il_Graphics_makeShader(GL_VERTEX_SHADER, 
                IL_ASSET_READFILE("heightmap.vert")),
              fragment = il_Graphics_makeShader(GL_FRAGMENT_SHADER,
                IL_ASSET_READFILE("heightmap.frag")),
              program = glCreateProgram();
      
      glAttachShader(program, vertex);
      glAttachShader(program, fragment);
      
      il_Graphics_linkProgram(program);
      
      ter->program = program;
      ter->draw = &heightmap_draw;
      break;
    }
    case PHEIGHTMAP: {
      struct pheightmap* pheightmap = parent->data;
      ter->buf = calloc(3, sizeof(GLuint));
      
      glGenVertexArrays(1, &ter->buf[2]);
      glBindVertexArray(ter->buf[2]);
      il_Graphics_testError("Failed to create VAO");
      
      glGenBuffers(2, ter->buf);
      glBindBuffer(GL_ARRAY_BUFFER, ter->buf[0]);
      int width = pheightmap->viewdistance / pheightmap->resolution,
          size = width * width,
          x, y,
          *buf = calloc(size, sizeof(int)*2); // vector2 = 2 floats
      for (y = 0; y < width; y++) {
        for (x = 0; x < width; x++) {
          buf[y*width*2 + 2*x + 0] = x;
          buf[y*width*2 + 2*x + 1] = y;
        }
      }
      glBufferData(GL_ARRAY_BUFFER, size * sizeof(int) * 2, buf, 
        GL_STATIC_DRAW);
      free(buf);
      il_Graphics_testError("Failed to create VBO");
      
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ter->buf[1]);
      int rowsize = width*2 + 1; // width*2 indices, 1 end restart index
      buf = calloc((width-1) * rowsize, sizeof(int));
      // width-1 columns, because tiles are "in-between" vertices
      for (y = 0; y < width-1; y++) {
        for (x = 0; x < width-1; x++) {
          buf[y*rowsize + x*2 + 0] = y*width + x;
          buf[y*rowsize + x*2 + 1] = (y+1)*width + x;
        }
        buf[(y+1)*rowsize - 1] = -1; // restart index
      }
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, (width-1) * rowsize * sizeof(int), 
        buf, GL_STATIC_DRAW);
      free(buf);
      il_Graphics_testError("Failed to create IBO");
      
      GLuint  vertex = il_Graphics_makeShader(GL_VERTEX_SHADER, 
                IL_ASSET_READFILE("pheightmap.vert")),
              fragment = il_Graphics_makeShader(GL_FRAGMENT_SHADER,
                IL_ASSET_READFILE("pheightmap.frag")),
              program = glCreateProgram();
      
      glAttachShader(program, vertex);
      glAttachShader(program, fragment);
      
      il_Graphics_linkProgram(program);
      
      glBindBuffer(GL_ARRAY_BUFFER, ter->buf[0]);
      il_Graphics_testError("glBindBuffer");
      GLint in_pos = glGetAttribLocation(program, "in_Position");
      il_Graphics_testError("glGetAttribLocation");
      // index 0, 2 components, integer, non-normalized, tightly packed, 
      // no offset
      glVertexAttribPointer(0, 2, GL_INT, GL_FALSE, 0, (GLvoid*)0);
      il_Graphics_testError("glVertexAttribPointer");
      glEnableVertexAttribArray(in_pos);
      il_Graphics_testError("Failed to bind vertex attributes");
      
      ter->program = program;
      ter->draw = &pheightmap_draw;
      ter->draw_ctx = pheightmap;
      break;
    }
    case NONE:
    default:
      // ???
      break;
  }
  
  il_Graphics_Drawable3d_setPositionable(&ter->drawable, positionable);
  
  return ter;
}

