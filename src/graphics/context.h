#ifndef ILG_CONTEXT_H
#define ILG_CONTEXT_H

#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <GL/glew.h>

typedef struct ilG_context {
    struct ilG_drawable3d* drawable;
    struct ilG_material* material;
    struct ilG_texture* texture;
    struct ilG_camera* camera;
    struct il_world* world;
    struct timeval tv;
    unsigned *texunits;
    size_t num_texunits;
    size_t num_active;
} ilG_context;

ilG_context* ilG_context_new();

void ilG_context_setActive(ilG_context*);

#endif

