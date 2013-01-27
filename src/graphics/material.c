#include "material.h"

#include <GL/glew.h>
#include <string.h>

#include "graphics/glutil.h"
#include "common/string.h"
#include "asset/asset.h"
#include "graphics/context.h"
#include "graphics/textureunit.h"
#include "graphics/arrayattrib.h"
#include "common/log.h"
#include "graphics/drawable3d.h"
#include "graphics/tracker.h"

struct generic_material {
    ilG_material mtl; // oh how I wish the plan9 functionality was in C11
    const char **unitlocs;
    unsigned long *unittypes;
    const char *mvp;
};

static void mtl_bind(ilG_context* ctx, void * user)
{
    (void)user;
    struct generic_material* mtl = (struct generic_material*)ctx->material;
    ilG_testError("Unknown");
    glUseProgram(mtl->mtl.program);
    ilG_testError("glUseProgram()");
    unsigned int i = 0;
    while (mtl->unitlocs[i]) {
        GLint loc = glGetUniformLocation(mtl->mtl.program, mtl->unitlocs[i]);
        ilG_testError("glGetUniformLocation()");
        glUniform1i(loc, i);
        ilG_testError("glUniform1i()");
        ILG_TUNIT_ACTIVE(ctx, i, mtl->unittypes[i]);
        i++;
    }
}

static void mtl_update(ilG_context* context, struct il_positionable* pos, void *ctx)
{
    (void)ctx;
    struct generic_material* mtl = (struct generic_material*)context->material;
    if ((mtl->mtl.attrs & context->drawable->attrs) != mtl->mtl.attrs) {
        il_log( 1, "Drawable \"%s\" does not have the required attributes to "
                "be drawn with Material \"%s\"", context->drawable->name, 
                mtl->mtl.name);
    }
    ilG_bindMVP(mtl->mvp, mtl->mtl.program, context->camera, pos);
}

static void mtl_unbind(ilG_context* context, void *ctx)
{
    (void)ctx;
    context->num_active = 0;
}

const char *strdup(const char*);
// all parameters to this function may be temporary
ilG_material* ilG_material_new(il_string vertsource, il_string fragsource, 
    const char *name, const char *position, const char *texcoord,
    const char *normal, const char *mvp, const char **unitlocs, 
    unsigned long *unittypes)
{
    struct generic_material* mtl;
   
    mtl = calloc(1, sizeof(struct generic_material));
    if (name)   mtl->mtl.name = strdup(name);
    else        mtl->mtl.name = "(null)";
    mtl->mtl.vertshader = ilG_makeShader(GL_VERTEX_SHADER, vertsource);
    mtl->mtl.fragshader = ilG_makeShader(GL_FRAGMENT_SHADER, fragsource);
    mtl->mtl.program = glCreateProgram();
    glAttachShader(mtl->mtl.program, mtl->mtl.vertshader);
    glAttachShader(mtl->mtl.program, mtl->mtl.fragshader);
    if (position) {
        glBindAttribLocation(mtl->mtl.program, ILG_ARRATTR_POSITION, position);
        ILG_SETATTR(mtl->mtl.attrs, ILG_ARRATTR_POSITION);
    }
    if (texcoord) {
        glBindAttribLocation(mtl->mtl.program, ILG_ARRATTR_TEXCOORD, texcoord);
        ILG_SETATTR(mtl->mtl.attrs, ILG_ARRATTR_TEXCOORD);
    }
    if (normal) {
        glBindAttribLocation(mtl->mtl.program, ILG_ARRATTR_NORMAL, normal);
        ILG_SETATTR(mtl->mtl.attrs, ILG_ARRATTR_NORMAL);
    }
    ilG_linkProgram(mtl->mtl.program);
    unsigned int i = 0;
    while (unitlocs[i]) {
        i++;
    }
    const char **unitlocs_ = calloc(i+1, sizeof(const char*));
    unsigned long *unittypes_ = calloc(i, sizeof(unsigned long));
    memcpy(unittypes_, unittypes, sizeof(unsigned long) * i);
    unsigned int j;
    for (j = 0; j < i; j++) {
        unitlocs_[j] = strdup(unitlocs[j]);
    }
    mtl->unitlocs   = unitlocs_;
    mtl->unittypes  = unittypes_;
    mtl->mvp        = strdup(mvp);
    mtl->mtl.bind   = &mtl_bind;
    mtl->mtl.update = &mtl_update;
    mtl->mtl.unbind = &mtl_unbind;

    ilG_material_assignId(&mtl->mtl);

    return &mtl->mtl;
}

static void bind(ilG_context* context, void *ctx)
{
    (void)ctx;
    ilG_testError("Unknown");
    glUseProgram(context->material->program);
    ilG_testError("glUseProgram()");

    GLint loc = glGetUniformLocation(context->material->program, "tex");
    ilG_testError("glGetUniformLocation()");
    glUniform1i(loc, 0);
    ilG_testError("glUniform1i()");
    ILG_TUNIT_ACTIVE(context, 0, ILG_TUNIT_COLOR0);
}

static void update(ilG_context* context, struct il_positionable* pos, void *ctx)
{
    (void)ctx;
    if (!ILG_TESTATTR(context->drawable->attrs, ILG_ARRATTR_POSITION) ||
        !ILG_TESTATTR(context->drawable->attrs, ILG_ARRATTR_TEXCOORD)) {
        il_log( 1, "Drawable \"%s\" does not have the required attributes to "
                "be drawn with Material \"%s\"", context->drawable->name, 
                context->material->name);
    }
    ilG_bindMVP("mvp", context->material->program, context->camera, pos);
}

static void unbind(ilG_context* context, void *ctx)
{
    (void)ctx;
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

    ilG_testError("Unknown error before function");

    mtl.vertshader = ilG_makeShader(GL_VERTEX_SHADER, vertex_source);
    mtl.fragshader = ilG_makeShader(GL_FRAGMENT_SHADER, fragment_source);

    mtl.program = glCreateProgram();
    ilG_testError("Unable to create program object");

    glAttachShader(mtl.program, mtl.vertshader);
    ilG_testError("Unable to attach shader");
    glAttachShader(mtl.program, mtl.fragshader);
    ilG_testError("Unable to attach shader");

    glBindAttribLocation(mtl.program, ILG_ARRATTR_POSITION, "in_Position");
    glBindAttribLocation(mtl.program, ILG_ARRATTR_TEXCOORD, "in_Texcoord");

    ilG_linkProgram(mtl.program);

    mtl.bind = &bind;
    mtl.update = &update;
    mtl.unbind = &unbind;

    ilG_material_default = &mtl;
}

