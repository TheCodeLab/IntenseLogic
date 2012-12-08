#ifndef ILG_CONTEXT_H
#define ILG_CONTEXT_H

#include "graphics/drawable3d.h"
#include "graphics/material.h"
#include "graphics/texture.h"
#include "graphics/camera.h"
#include "common/world.h"

typedef struct ilG_context {
    ilG_drawable3d* current_drawable;
    ilG_material* current_material;
    ilG_texture* current_texture;
    ilG_camera* camera;
    il_world* world;
} ilG_context;

#endif

