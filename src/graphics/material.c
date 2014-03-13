#include "material.h"

#include <GL/glew.h>
#include <string.h>

#include "graphics/glutil.h"
#include "graphics/context.h"
#include "graphics/arrayattrib.h"
#include "util/log.h"
#include "graphics/drawable3d.h"
#include "graphics/fragdata.h"
#include "util/array.h"
#include "asset/node.h"
#include "util/ilassert.h"
#include "graphics/graphics.h"

struct textureunit {
    char *location;
    GLint uniform;
    unsigned long type;
};

struct matrixinfo {
    char *location;
    GLint uniform;
    enum ilG_transform type;
};

struct bindfunc {
    char *location;
    ilG_material_onBindFunc func;
    GLint uniform;
    void *user;
};

struct posfunc {
    char *location;
    ilG_material_onPosFunc func;
    GLint uniform;
    void *user;
};

struct ilG_material_config {
    IL_ARRAY(struct textureunit,) texunits;    
    IL_ARRAY(struct matrixinfo,) matrices;
    IL_ARRAY(struct bindfunc,) bindfuncs;
    IL_ARRAY(struct posfunc,) posfuncs;
    il_string *vertsource;
    il_string *fragsource;
    char* attriblocs[ILG_ARRATTR_NUMATTRS];
    char* fraglocs[ILG_FRAGDATA_NUMATTRS];
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
    }
    for (i = 0; i < mtl->config->matrices.length; i++) {
        if (!(mtl->config->matrices.data[i].type & ILG_MODEL)) { 
            // same for any positionable, otherwise it goes in update()
            ilG_bindMVP(mtl->config->matrices.data[i].uniform, mtl->config->matrices.data[i].type, mtl->context->camera, NULL);
        }
    }
    for (i = 0; i < mtl->config->bindfuncs.length; i++) {
        mtl->config->bindfuncs.data[i].func(mtl, mtl->config->bindfuncs.data[i].uniform, mtl->config->bindfuncs.data[i].user);
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
                 (void*)mtl->context->drawable,
                 il_typeof(mtl)->name, (void*)mtl);
    }
    unsigned int i;
    for (i = 0; i < mtl->config->matrices.length; i++) {
        if (mtl->config->matrices.data[i].type & ILG_MODEL) {
            // otherwise it was bound in bind()
            ilG_bindMVP(mtl->config->matrices.data[i].uniform, mtl->config->matrices.data[i].type, mtl->context->camera, mtl->context->positionable);
        }
    }
    for (i = 0; i < mtl->config->posfuncs.length; i++) {
        mtl->config->posfuncs.data[i].func(mtl, mtl->context->positionable, mtl->config->posfuncs.data[i].uniform, mtl->config->posfuncs.data[i].user);
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
    mtl->config = calloc(1, sizeof(struct ilG_material_config));
}

void ilG_material_vertex(ilG_material* self, il_string *source) 
{
    if (self->config->vertsource) {
        il_string_unref(self->config->vertsource);
    }
    self->config->vertsource = il_string_ref(source);
}

void ilG_material_vertex_file(ilG_material *self, const char *filename)
{
    size_t size;
    void *data;
    ilA_path *path = ilA_path_chars(filename);
    il_base *base = ilA_lookup(ilG_shaders_iface, ilG_shaders_dir, path);
    ilA_path_free(path);
    if (!base) {
        il_error("No such file %s", filename);
        return;
    }
    data = ilA_contents(NULL, base, &size);
    if (!data) {
        il_error("%s is not a file", filename);
        return;
    }
    ilG_material_vertex(self, il_string_bin(data, size));
    il_unref(base);
}

void ilG_material_fragment(ilG_material* self, il_string *source)
{
    if (self->config->fragsource) {
        il_string_unref(self->config->fragsource);
    }
    self->config->fragsource = il_string_ref(source);
}

void ilG_material_fragment_file(ilG_material *self, const char *filename)
{
    size_t size;
    void *data;
    ilA_path *path = ilA_path_chars(filename);
    il_base *base = ilA_lookup(ilG_shaders_iface, ilG_shaders_dir, path);
    ilA_path_free(path);
    if (!base) {
        il_error("No such file %s", filename);
        return;
    }
    data = ilA_contents(NULL, base, &size);
    if (!data) {
        il_error("%s is not a file", filename);
        return;
    }
    ilG_material_fragment(self, il_string_bin(data, size));
    il_unref(base);
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

void ilG_material_bindFunc(ilG_material* self, ilG_material_onBindFunc func, void *user, const char *location)
{
    struct bindfunc f = (struct bindfunc) {
        strdup(location),
        func,
        0,
        user
    };
    IL_APPEND(self->config->bindfuncs, f);
}

void ilG_material_posFunc(ilG_material* self, ilG_material_onPosFunc func, void *user, const char *location)
{
    struct posfunc f = (struct posfunc) {
        strdup(location),
        func,
        0,
        user
    };
    IL_APPEND(self->config->posfuncs, f);
}

static void link(ilG_material *self)
{
    ilG_testError("Unknown");
    il_log("Building shader \"%s\"", self->name);
    self->vertshader = ilG_makeShader(GL_VERTEX_SHADER, self->config->vertsource);
    self->fragshader = ilG_makeShader(GL_FRAGMENT_SHADER, self->config->fragsource);
    self->program = glCreateProgram();
    /*if (GLEW_KHR_debug) {
        glObjectLabel(GL_PROGRAM, self->program, strlen(self->name), self->name);
        char buf[1024];
        glGetObjectLabel(GL_PROGRAM, self->program, 1024, NULL, buf);
        printf("Program %u labelled %s\n", self->program, buf);
    }*/
    if (!glIsShader(self->vertshader) || !glIsShader(self->fragshader) || !glIsProgram(self->program)) {
        self->valid = 0;
        return;
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
    for (i = 0; i < self->config->bindfuncs.length; i++) {
        self->config->bindfuncs.data[i].uniform = glGetUniformLocation(self->program, self->config->bindfuncs.data[i].location);
    }
    for (i = 0; i < self->config->posfuncs.length; i++) {
        self->config->posfuncs.data[i].uniform = glGetUniformLocation(self->program, self->config->posfuncs.data[i].location);
    }

    self->valid = 1;
}

int /*failure*/ ilG_material_link(ilG_material* self, ilG_context *ctx)
{
    if (!ctx) {
        il_error("Null context");
    }
    if (!self) {
        il_error("Null material");
    }
    self->context = ctx;
    link(self);
    return 0;
}

il_type ilG_material_type = {
    .typeclasses = NULL,
    .storage = {NULL},
    .constructor = material_init,
    .destructor = NULL,
    .copy = NULL,
    .name = "il.graphics.material",
    .size = sizeof(ilG_material),
    .parent = NULL
};

struct ilG_bindable ilG_material_bindable = {
    .name = "il.graphics.bindable",
    .bind = mtl_bind,
    .action = mtl_update,
    .unbind = mtl_unbind
};

ilG_material ilG_material_default;

void ilG_material_init()
{
    il_impl(&ilG_material_type, &ilG_material_bindable);
}

