#include "material.h"

#include <GL/glew.h>
#include <string.h>

#include "graphics/glutil.h"
#include "asset/asset.h"
#include "graphics/context.h"
#include "graphics/textureunit.h"
#include "graphics/arrayattrib.h"
#include "util/log.h"
#include "graphics/drawable3d.h"
#include "graphics/tracker.h"
#include "graphics/fragdata.h"
#include "common/array.h"
#include "asset/asset.h"

struct textureunit {
    const char *location;
    GLint uniform;
    unsigned long type;
};

struct matrixinfo {
    const char *location;
    GLint uniform;
    enum ilG_transform type;
};

struct ilG_material_config {
    IL_ARRAY(struct textureunit,) texunits;    
    IL_ARRAY(struct matrixinfo,) matrices;
    il_string *vertsource;
    il_string *fragsource;
    const char* attriblocs[ILG_ARRATTR_NUMATTRS];
    const char* fraglocs[ILG_FRAGDATA_NUMATTRS];
};

static void mtl_bind(ilG_context* ctx, void * user)
{
    (void)user;
    ilG_material* mtl = ctx->material;
    if (!mtl->valid) { // silently fail so we don't flood the console, the error has already been printed
        return;
    }
    ilG_testError("Unknown");
    glUseProgram(mtl->program);
    ilG_testError("glUseProgram()");
    unsigned int i;
    for (i = 0; i < mtl->config->texunits.length; i++) {
        glUniform1i(mtl->config->texunits.data[i].uniform, i);
        ilG_testError("glUniform1i()");
        ILG_TUNIT_ACTIVE(ctx, i, mtl->config->texunits.data[i].type);
    }
    for (i = 0; i < mtl->config->matrices.length; i++) {
        if (!(mtl->config->matrices.data[i].type & ILG_MODEL)) { 
            // same for any positionable, otherwise it goes in update()
            ilG_bindMVP(mtl->config->matrices.data[i].uniform, mtl->config->matrices.data[i].type, ctx->camera, NULL);
        }
    }
}

static void mtl_update(ilG_context* context, struct il_positionable* pos, void *ctx)
{
    (void)ctx;
    ilG_material* mtl = context->material;
    if (!mtl->valid) {
        return;
    }
    if ((mtl->attrs & context->drawable->attrs) != mtl->attrs) {
        il_error("Drawable \"%s\" does not have the required attributes to "
                 "be drawn with Material \"%s\"", context->drawable->name, 
                 mtl->name);
    }
    unsigned int i;
    for (i = 0; i < mtl->config->matrices.length; i++) {
        if (mtl->config->matrices.data[i].type & ILG_MODEL) {
            // otherwise it was bound in bind()
            ilG_bindMVP(mtl->config->matrices.data[i].uniform, mtl->config->matrices.data[i].type, context->camera, pos);
        }
    }
}

static void mtl_unbind(ilG_context* context, void *ctx)
{
    (void)ctx;
    context->num_active = 0;
}

char *strdup(const char*);

ilG_material* ilG_material_new()
{
    ilG_material* mtl = calloc(1, sizeof(ilG_material));
    ilG_material_assignId(mtl);
    mtl->name = "Unnamed";
    mtl->config = calloc(1, sizeof(struct ilG_material_config));
    return mtl;
}

void ilG_material_vertex(ilG_material* self, il_string *source) 
{
    self->config->vertsource = il_string_ref(source);
}

void ilG_material_fragment(ilG_material* self, il_string *source)
{
    self->config->fragsource = il_string_ref(source);
}

void ilG_material_name(ilG_material* self, const char* name)
{
    self->name = strdup(name);
}

void ilG_material_arrayAttrib(ilG_material* self, unsigned long attrib, const char *location) 
{
    self->config->attriblocs[attrib] = strdup(location);
}

void ilG_material_fragData(ilG_material* self, unsigned long attrib, const char *location)
{
    self->config->fraglocs[attrib] = strdup(location);
}

void ilG_material_textureUnit(ilG_material* self, unsigned long unittype, const char *location)
{
    struct textureunit unit = (struct textureunit){
        strdup(location),
        0,//glGetUniformLocation(self->program, location),
        unittype
    };
    IL_APPEND(self->config->texunits, unit);
}

void ilG_material_matrix(ilG_material* self, enum ilG_transform type, const char *location)
{
    struct matrixinfo mat = (struct matrixinfo){
        strdup(location),
        0,//glGetUniformLocation(self->program, location),
        type
    };
    IL_APPEND(self->config->matrices, mat);
}

int /*failure*/ ilG_material_link(ilG_material* self)
{
    ilG_testError("Unknown");
    il_log("Building shader \"%s\"", self->name);
    self->vertshader = ilG_makeShader(GL_VERTEX_SHADER, self->config->vertsource);
    self->fragshader = ilG_makeShader(GL_FRAGMENT_SHADER, self->config->fragsource);
    self->program = glCreateProgram();
    if (!glIsShader(self->vertshader) || !glIsShader(self->fragshader) || !glIsProgram(self->program)) {
        self->valid = 0;
        return 1;
    }
    glAttachShader(self->program, self->vertshader);
    glAttachShader(self->program, self->fragshader);
    ilG_testError("glAttachShader");
    unsigned int i;
    for (i = 0; i < ILG_ARRATTR_NUMATTRS; i++) {
        if (self->config->attriblocs[i]) {
            glBindAttribLocation(self->program, i, self->config->attriblocs[i]);
            ILG_SETATTR(self->attrs, i);
        }
    }
    ilG_testError("Error binding array attributes");
    for (i = 0; i < ILG_FRAGDATA_NUMATTRS; i++) {
        if (self->config->fraglocs[i]) {
            glBindFragDataLocation(self->program, i, self->config->fraglocs[i]);
        }
    }
    ilG_testError("Error binding fragment outputs");
    ilG_linkProgram(self->program);
    for (i = 0; i < self->config->texunits.length; i++) {
        self->config->texunits.data[i].uniform = glGetUniformLocation(self->program, self->config->texunits.data[i].location);
    }
    ilG_testError("Error binding texture units");
    for (i = 0; i < self->config->matrices.length; i++) {
        self->config->matrices.data[i].uniform = glGetUniformLocation(self->program, self->config->matrices.data[i].location);
    }
    ilG_testError("Error binding matrices");

    self->bind = &mtl_bind;
    self->update = &mtl_update;
    self->unbind = &mtl_unbind;
    self->valid = 1;
    return 0;
}

/*ilG_material* ilG_material_new(il_string vertsource, il_string fragsource, 
    const char *name, const char *position, const char *texcoord,
    const char *normal, const char *mvp, const char **unitlocs, 
    unsigned long *unittypes, const char *normalOut, const char *ambient, 
    const char *diffuse, const char *specular, const char *phong)
{
    struct generic_material* mtl;
   
    mtl = calloc(1, sizeof(struct generic_material));
    if (name)   mtl->mtl.name = strdup(name);
    else        mtl->mtl.name = "(null)";
    il_log(3, "Building shader \"%s\"", mtl->mtl.name);
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
    if (ambient) {
        glBindFragDataLocation(mtl->mtl.program, ILG_FRAGDATA_ACCUMULATION, ambient);
    }
    if (diffuse) {
        glBindFragDataLocation(mtl->mtl.program, ILG_FRAGDATA_DIFFUSE, diffuse);
    }
    if (specular) {
        glBindFragDataLocation(mtl->mtl.program, ILG_FRAGDATA_SPECULAR, specular);
    }
    if (normalOut) {
        glBindFragDataLocation(mtl->mtl.program, ILG_FRAGDATA_NORMAL, normalOut);
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
    mtl->mvp        = mvp? strdup(mvp) : NULL;
    mtl->phong      = phong? strdup(phong) : NULL;
    mtl->mtl.bind   = &mtl_bind;
    mtl->mtl.update = &mtl_update;
    mtl->mtl.unbind = &mtl_unbind;

    ilG_material_assignId(&mtl->mtl);

    return &mtl->mtl;
}*/

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
        il_error("Drawable \"%s\" does not have the required attributes to "
                 "be drawn with Material \"%s\"", context->drawable->name, 
                 context->material->name);
    }
    ilG_bindMVP(glGetUniformLocation(context->material->program, "mvp"), ILG_MVP, context->camera, pos);
}

static void unbind(ilG_context* context, void *ctx)
{
    (void)ctx;
    context->num_active = 0;
}

static ilG_material mtl;

void ilG_material_init()
{
    il_string *vertex_source, *fragment_source;

    memset(&mtl, 0, sizeof(ilG_material));
    ilG_material_assignId(&mtl);
    mtl.name = "Default";

    vertex_source = IL_ASSET_READFILE("shaders/default.vert");
    fragment_source = IL_ASSET_READFILE("shaders/default.frag");

    if (!vertex_source) {
        il_error("Unable to open cube vertex shader");
        return;
    }
    if (!fragment_source) {
        il_error("Unable to open cube fragment shader");
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

