#include "terrain.h"

#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>

#include "graphics/drawable3d.h"
#include "asset/asset.h"
#include "common/string.h"
#include "common/log.h"

enum terrain_type {
  NONE = 0,
  HEIGHTMAP,
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
  double (*getPoint)(il_Common_Terrain*, void*, unsigned x, unsigned y, double height);
  void *normal_ctx;
  sg_Vector3 (*getNormal)(il_Common_Terrain*, void*, unsigned x, unsigned y, double z);
};

struct il_Graphics_Terrain {
  il_Graphics_Drawable3d drawable;
  il_Common_Terrain * terrain;
  GLuint *buf;
  GLuint program;
  void *draw_ctx;
  void (*draw)(il_Graphics_Terrain*, void*, const il_Graphics_Camera*, const struct timeval*);
};

static const char * read_file(const char *name) {
  return il_Common_toC(il_Asset_readContents(il_Asset_open(il_Common_fromC((char*)name))));
}

static void terrain_draw(const il_Graphics_Camera* cam, struct il_Graphics_Drawable3d* drawable, const struct timeval* tv) {
  il_Graphics_Terrain* ter = (il_Graphics_Terrain*)drawable;
  ter->draw(ter, ter->draw_ctx, cam, tv);
}

static void heightmap_draw(il_Graphics_Terrain* ter, void* ctx, const il_Graphics_Camera* cam, const struct timeval* tv) {
  glUseProgram(ter->program);
  
  //activate the heightmap texture
  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D, ter->buf[0]);
  
  glDrawArrays(GL_TRIANGLE_FAN, 0, ter->terrain->width * ter->terrain->height);
}

il_Graphics_Terrain* il_Graphics_Terrain_new(il_Common_Terrain* parent, il_Common_Positionable* positionable) {
  il_Graphics_Terrain * ter = calloc(sizeof(il_Graphics_Terrain),1);
  ter->terrain = parent;
  ter->drawable.positionable = positionable;
  ter->drawable.draw = (il_Graphics_Drawable3d_cb)&terrain_draw;
  switch(parent->type) {
    case NONE:
      // ???
      break;
    case HEIGHTMAP: {
      glGenTextures(1, ter->buf);
      glBindTexture(GL_TEXTURE_2D, ter->buf[0]);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, parent->width, parent->height, 0, GL_RED, GL_FLOAT, parent->data);
      
      GLuint vertex, fragment;
      vertex = glCreateShader(GL_VERTEX_SHADER);
      fragment = glCreateShader(GL_FRAGMENT_SHADER);
      const GLchar *vertex_source, *fragment_source;
      vertex_source = read_file("heightmap.vert");
      fragment_source = read_file("heightmap.frag");
      glShaderSource(vertex, 1, &vertex_source, 0);
      glShaderSource(fragment, 1, &fragment_source, 0);
      free((void*)vertex_source);
      free((void*)fragment_source);
      
      glCompileShader(vertex);
      glCompileShader(fragment);
      
      int status;
      glGetShaderiv(vertex, GL_COMPILE_STATUS, &status);
      if (status == GL_FALSE) {
        int maxLength;
        glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &maxLength);
        char *vertexInfoLog = (char *)calloc(1, maxLength);
   
        glGetShaderInfoLog(vertex, maxLength, &maxLength, vertexInfoLog);
        
        il_Common_log(1, "Compilation failed for heightmap.vert: %s", vertexInfoLog);
        
        free(vertexInfoLog);
        return NULL;
      }
      glGetShaderiv(fragment, GL_COMPILE_STATUS, &status);
      if (status == GL_FALSE) {
        int maxLength;
        glGetShaderiv(fragment, GL_INFO_LOG_LENGTH, &maxLength);
        char *fragmentInfoLog = (char *)calloc(1, maxLength);
   
        glGetShaderInfoLog(fragment, maxLength, &maxLength, fragmentInfoLog);
        
        il_Common_log(1, "Compilation failed for heightmap.frag: %s", fragmentInfoLog);
        
        free(fragmentInfoLog);
        return NULL;
      }
      
      ter->draw = &heightmap_draw;
      break;
    }
  }
  return ter;
}

