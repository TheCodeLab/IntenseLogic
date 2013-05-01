#ifndef ILG_CONTEXT_H
#define ILG_CONTEXT_H

#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "util/array.h"
#include "graphics/light.h"

struct ilG_stage;

typedef struct ilG_context {
    struct ilG_drawable3d* drawable;
    struct ilG_material* material;
    struct ilG_texture* texture;
    struct ilG_camera* camera;
    struct il_world* world;
    struct il_positionable *positionable;
    struct timeval tv;
    unsigned *texunits;
    size_t num_texunits;
    size_t num_active;
    IL_ARRAY(struct il_positionable*,) positionables; // tracker.c
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
    int which;
    IL_ARRAY(struct ilG_stage*,) stages;
} ilG_context;

ilG_context* ilG_context_new(int w, int h);

void ilG_context_setActive(ilG_context*);
void ilG_context_addStage(ilG_context* self, struct ilG_stage* stage, int num);

#endif

