#ifndef ILG_SKYBOXPASS_H
#define ILG_SKYBOXPASS_H

#include "graphics/stage.h"

struct ilG_texture;
struct ilG_context;

typedef struct ilG_skybox ilG_skybox;

extern const ilG_stagable ilG_skybox_stage;

ilG_skybox *ilG_skybox_new(struct ilG_context *context, struct ilG_texture *skytex);

#endif

