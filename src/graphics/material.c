#include "material.h"

#include <GL/glew.h>

#include "graphics/glutil.h"
#include "common/string.h"
#include "asset/asset.h"
#include "graphics/context.h"
#include "graphics/textureunit.h"

static void bind(ilG_context* context, void *ctx)
{
    IL_GRAPHICS_TESTERROR("Unknown");
    glUseProgram(context->material->program);
    IL_GRAPHICS_TESTERROR("glUseProgram()");

    GLint loc = glGetUniformLocation(context->material->program, "tex");
    IL_GRAPHICS_TESTERROR("glGetUniformLocation()");
    glUniform1i(loc, 0);
    IL_GRAPHICS_TESTERROR("glUniform1i()");
    ILG_TUNIT_ACTIVE(context, 0, ILG_TUNIT_COLOR0);
}

static void update(ilG_context* context, struct il_positionable* pos, void *ctx)
{
    (void)ctx;
    ilG_bindMVP("mvp", context->material->program, context->camera, pos);
}

static void unbind(ilG_context* context, void *ctx)
{
    context->num_active = 0;
}

static ilG_material mtl;

void ilG_material_init()
{
    il_string vertex_source, fragment_source;

    memset(&mtl, 0, sizeof(ilG_material));
    ilG_material_assignId(&mtl);
    mtl.name = "Default";

    vertex_source = IL_ASSET_READFILE("shaders/default.vert");
    fragment_source = IL_ASSET_READFILE("shaders/default.frag");

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

    glBindAttribLocation(mtl.program, 0, "in_Position");
    glBindAttribLocation(mtl.program, 1, "in_Texcoord");

    ilG_linkProgram(mtl.program);

    mtl.bind = &bind;
    mtl.update = &update;

    ilG_material_default = &mtl;
}

