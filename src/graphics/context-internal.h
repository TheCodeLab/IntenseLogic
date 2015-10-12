#ifndef ILG_CONTEXT_INTERNAL_H
#define ILG_CONTEXT_INTERNAL_H

#include <pthread.h>

#include "graphics/renderer.h"
#include "graphics/transform.h"
#include "math/matrix.h"

void ilG_default_update(void *, ilG_rendid);
void ilG_default_multiupdate(void *, ilG_rendid, il_mat *);
void ilG_default_draw(void *, ilG_rendid, il_mat **, const unsigned*, unsigned);
void ilG_default_viewmats(void*, il_mat*, int*, unsigned);
void ilG_default_objmats(void*, const unsigned*, unsigned, il_mat*, int);
void ilG_default_free(void*);

#endif
