#ifndef ILG_SKYBOXPASS_H
#define ILG_SKYBOXPASS_H

#include "graphics/renderer.h"

struct ilG_texture;
struct ilG_context;

typedef struct ilG_skybox ilG_skybox;

extern const ilG_renderable ilG_skybox_renderer;

#define ilG_skybox_wrap(p) ilG_renderer_wrap(p, &ilG_skybox_renderer)

ilG_skybox *ilG_skybox_new(struct ilG_texture *skytex);

#endif

