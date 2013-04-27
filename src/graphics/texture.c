#include "texture.h"

#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>

#include "asset/asset.h"
#include "asset/texture.h"
#include "graphics/tracker.h"
#include "graphics/context.h"
#include "util/log.h"
#include "graphics/bindable.h"

static void texture_update(void *obj)
{
    unsigned int i;
    ilG_texture *tex = obj;

    if (tex->last_mtl == tex->context->material) {
        return;
    }
    tex->last_mtl = tex->context->material;
    for (i = 0; i < tex->context->num_active; i++) {
        if (tex->units[tex->context->texunits[i]].used) {
            glActiveTexture(i);
            glBindTexture(tex->units[tex->context->texunits[i]].mode, tex->units[tex->context->texunits[i]].tex);
        }
    }
}

static void texture_init(void *obj)
{
    ilG_texture *tex = obj;
    tex->name = "Unnamed";
    ilG_texture_assignId(tex);
}

void ilG_texture_setContext(ilG_texture* self, struct ilG_context *context)
{
    self->context = context;
}

void ilG_texture_setName(ilG_texture* self, const char *name)
{
    self->name = strdup(name);
}

void ilG_texture_fromfile(ilG_texture* self, unsigned unit, const char *name)
{
    ilA_asset* asset = ilA_open(il_string_new(name, strlen(name)));
    ilG_texture_fromasset(self, unit, asset);
}

void ilG_texture_fromasset(ilG_texture* self, unsigned unit, ilA_asset* asset)
{
    self->units[unit] = (ilG_textureunit) {
        .used = 1,
        .tex = ilA_assetToTexture(asset),
        .mode = GL_TEXTURE_2D
    };
}

void ilG_texture_fromGL(ilG_texture* self, unsigned unit, GLenum target, GLuint tex)
{
    self->units[unit] = (ilG_textureunit) {
        .used = 1,
        .tex = tex,
        .mode = target
    };
}

void ilG_texture_fromdata(ilG_texture* self, unsigned unit, GLenum target, 
    GLenum internalformat, unsigned width, unsigned height, unsigned depth, GLenum format, 
    GLenum type, void *data)
{
    GLuint tex;

    glGenTextures(target, &tex);
    glBindTexture(target, tex);
    switch (target) {
        case GL_TEXTURE_1D:
        case GL_PROXY_TEXTURE_1D:
        glTexImage1D(target, 0, internalformat, width, 0, format, type, data);
        break;
        case GL_TEXTURE_2D:
        case GL_PROXY_TEXTURE_2D:
        case GL_TEXTURE_1D_ARRAY:
        case GL_PROXY_TEXTURE_1D_ARRAY:
        case GL_TEXTURE_RECTANGLE:
        case GL_PROXY_TEXTURE_RECTANGLE:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        case GL_PROXY_TEXTURE_CUBE_MAP:
        glTexImage2D(target, 0, internalformat, width, height, 0, format, type, data);
        break;
        case GL_TEXTURE_3D:
        case GL_PROXY_TEXTURE_3D:
        case GL_TEXTURE_2D_ARRAY:
        case GL_PROXY_TEXTURE_2D_ARRAY:
        glTexImage3D(target, 0, internalformat, width, height, depth, 0, format, type, data);
        break;
        default:
        il_error("Unknown texture format");
        return;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    self->units[unit] = (ilG_textureunit) {
        .used = 1,
        .tex = tex,
        .mode = target
    };
}

GLuint ilG_texture_getTex(ilG_texture* self, unsigned unit, GLenum *target)
{
    if (target) {
        *target = self->units[unit].mode;
    }
    return self->units[unit].tex;
}

void ilG_texture_setFilter(ilG_texture* self, unsigned unit, GLenum min_filter, GLenum mag_filter)
{
    glBindTexture(self->units[unit].mode, self->units[unit].tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
}

GLuint ilG_texture_getRaw(ilG_texture *self, unsigned unit)
{
    return self->units[unit].tex;
}

il_type ilG_texture_type = {
    .typeclasses = NULL,
    .storage = NULL,
    .constructor = texture_init,
    .destructor = NULL,
    .copy = NULL,
    .name = "il.graphics.texture",
    .registry = NULL,
    .size = sizeof(ilG_texture),
    .parent = NULL
};

ilG_bindable texture_bindable = {
    .name = "il.graphics.bindable",
    .hh = {0},
    .bind = NULL,
    .action = texture_update,
    .unbind = NULL
};

void ilG_texture_init()
{
    il_impl(&ilG_texture_type, &texture_bindable);
}

