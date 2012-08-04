#ifndef IL_GRAPHICS_HEIGHTMAP_H
#define IL_GRAPHICS_HEIGHTMAP_H

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include "common/heightmap.h"
#include "drawable3d.h"
#include "common/matrix.h"

typedef struct il_Graphics_Heightmap {
	il_Graphics_Drawable3d drawable;
	il_Common_Heightmap *heightmap;
  GLuint vbo;
  GLsizei num_vertices;
} il_Graphics_Heightmap;

il_Graphics_Heightmap* il_Graphics_Heightmap_new(il_Common_Heightmap *heightmap);
void il_Graphics_Heightmap_redraw(il_Graphics_Heightmap *heightmap);

#endif
