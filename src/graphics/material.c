#include "material.h"

#include "graphics/glutil.h"

static void bind(ilG_material* material, void *ctx)
{
    glUseProgram(material->program);
}

void ilG_material_init()
{
    memset(&ilG_material_default, 0, sizeof(ilG_material));
    ilG_material_assignId(&ilG_material_default);
    ilG_material_default.name = "Default";
}

