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
#include "util/array.h"
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

static void mtl_bind(void *obj)
{
    ilG_material* mtl = obj;
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
        ILG_TUNIT_ACTIVE(mtl->context, i, mtl->config->texunits.data[i].type);
    }
    for (i = 0; i < mtl->config->matrices.length; i++) {
        if (!(mtl->config->matrices.data[i].type & ILG_MODEL)) { 
            // same for any positionable, otherwise it goes in update()
            ilG_bindMVP(mtl->config->matrices.data[i].uniform, mtl->config->matrices.data[i].type, mtl->context->camera, NULL);
        }
    }
}

static void mtl_update(void *obj)
{
    ilG_material* mtl = obj;
    if (!mtl->valid) {
        return;
    }
    if ((mtl->attrs & mtl->context->drawable->attrs) != mtl->attrs) {
        il_error("%s<%p> does not have the required attributes to "
                 "be drawn with %s<%p>", 
                 il_typeof(mtl->context->drawable)->name, 
                 mtl->context->drawable,
                 il_typeof(mtl)->name, mtl);
    }
    unsigned int i;
    for (i = 0; i < mtl->config->matrices.length; i++) {
        if (mtl->config->matrices.data[i].type & ILG_MODEL) {
            // otherwise it was bound in bind()
            ilG_bindMVP(mtl->config->matrices.data[i].uniform, mtl->config->matrices.data[i].type, mtl->context->camera, mtl->context->positionable);
        }
    }
}

static void mtl_unbind(void *obj)
{
    ilG_material *mtl = obj;
    mtl->context->num_active = 0;
}

char *strdup(const char*);

static void material_init(void *obj)
{
    ilG_material* mtl = obj;
    ilG_material_assignId(mtl);
    mtl->config = calloc(1, sizeof(struct ilG_material_config));
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

int /*failure*/ ilG_material_link(ilG_material* self, ilG_context *ctx)
{
    if (!ctx) {
        il_error("Null context");
    }
    if (!self) {
        il_error("Null material");
    }
    ilG_testError("Unknown");
    il_log("Building shader \"%s\"", self->name);
    self->context = ctx;
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

    self->valid = 1;
    return 0;
}

il_type ilG_material_type = {
    .typeclasses = NULL,
    .storage = NULL,
    .constructor = material_init,
    .destructor = NULL,
    .copy = NULL,
    .name = "il.graphics.material",
    .registry = NULL,
    .size = sizeof(ilG_material),
    .parent = NULL
};

static ilG_bindable material_bindable = {
    .name = "il.graphics.bindable",
    //.hh = {0},
    .bind = mtl_bind,
    .action = mtl_update,
    .unbind = mtl_unbind
};

ilG_material ilG_material_default;

void ilG_material_init()
{
    il_impl(&ilG_material_type, &material_bindable);

    // TODO: decide what to do with default material; as materials require a context value when being linked, and there may be multiple contexts
    /*memset(&ilG_material_default, 0, sizeof(ilG_material));
    il_init(&ilG_material_type, &ilG_material_default);
    ilG_material_name(&ilG_material_default, "Default");
    il_string *vertex = IL_ASSET_READFILE("default.vert"), *fragment = IL_ASSET_READFILE("default.frag");
    ilG_material_vertex(&ilG_material_default, vertex);
    ilG_material_fragment(&ilG_material_default, fragment);
    il_string_unref(vertex);
    il_string_unref(fragment);
    ilG_material_arrayAttrib(&ilG_material_default, ILG_ARRATTR_POSITION, "in_Position");
    ilG_material_arrayAttrib(&ilG_material_default, ILG_ARRATTR_TEXCOORD, "in_Texcoord");
    ilG_material_link(&ilG_material_default);*/
}

