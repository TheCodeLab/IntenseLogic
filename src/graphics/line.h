#ifndef ILG_LINE_H
#define ILG_LINE_H

#include "graphics/renderer.h"

// takes ownership of verts
// verts is 3*count floats, or number of vertices
ilG_builder ilG_line_builder(unsigned num, float *verts, const float col[3]);

#endif
