#include "material.h"

#include "graphics/glutil.h"
#include "common/string.h"
#include "asset/asset.h"

static void bind(ilG_material* material, void *ctx)
{
    glUseProgram(material->program);
}

static ilG_material mtl;

void ilG_material_init()
{
    il_string vertex_source, fragment_source;

    memset(&mtl, 0, sizeof(ilG_material));
    ilG_material_assignId(&mtl);
    mtl.name = "Default";

    vertex_source = IL_ASSET_READFILE("default.vert");
    fragment_source = IL_ASSET_READFILE("default.frag");

    if (!vertex_source.length) {
        il_log(1, "Unable to open cube vertex shader");
        return;
    }
    if (!fragment_source.length) {
        il_log(1, "Unable to open cube fragment shader");
        return;
    }

    IL_GRAPHICS_TESTERROR("Unknown error before function");

    mtl.vertshader = ilG_makeShader(GL_VERTEX_SHADER, vertex_source);
    mtl.fragshader = ilG_makeShader(GL_FRAGMENT_SHADER, fragment_source);

    mtl.program = glCreateProgram();
    IL_GRAPHICS_TESTERROR("Unable to create program object");

    glAttachShader(mtl.program, mtl.vertshader);
    IL_GRAPHICS_TESTERROR("Unable to attach shader");
    glAttachShader(mtl.program, mtl.fragshader);
    IL_GRAPHICS_TESTERROR("Unable to attach shader");

    ilG_linkProgram(mtl.program);

    mtl.bind = &bind;

    ilG_material_default = &mtl;
}

