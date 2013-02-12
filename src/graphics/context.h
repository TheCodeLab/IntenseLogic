#ifndef ILG_CONTEXT_H
#define ILG_CONTEXT_H

#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "common/array.h"
#include "graphics/light.h"

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
    IL_ARRAY(struct il_positionable*,) positionables; // tracker.c
    ilG_phong phong;
    IL_ARRAY(ilG_light*,) lights;
    GLuint fbtextures[5], framebuffer; // depth, accumulation, normal, diffuse, specular
    int width, height;
    struct {
        GLuint vao, vbo, ibo, lights_ubo, lights_index, mvp_ubo, mvp_index;
        GLint lights_size, mvp_size, lights_offset[3], mvp_offset[1];
        struct ilG_material* material;
        int invalidated;
        int created;
    } lightdata;
} ilG_context;

ilG_context* ilG_context_new(int w, int h);

void ilG_context_setActive(ilG_context*);

#endif

