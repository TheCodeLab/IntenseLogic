#ifndef ILG_LINE_H
#define ILG_LINE_H

#include "graphics/renderer.h"

// num is number of line segments, not number of vertices; takes ownership of verts.
// verts is 3*2*count floats
ilG_builder ilG_line_builder(unsigned num, float *verts, const float col[3]);

#endif
