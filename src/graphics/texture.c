#include "texture.h"

#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>

#include "asset/node.h"
#include "asset/image.h"
#include "graphics/tracker.h"
#include "graphics/context.h"
#include "util/log.h"
#include "graphics/bindable.h"

char *strdup(const char*);

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
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(tex->units[tex->context->texunits[i]].mode, tex->units[tex->context->texunits[i]].tex);
        }
    }
}

static void texture_unbind(void *obj)
{
    ilG_texture *tex = obj;

    tex->last_mtl = NULL;
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
    ilA_img *img = ilA_img_loadfile(name);
    if (!img) {
        return;
    }
    ilG_texture_fromimage(self, unit, img);
    ilA_img_free(img);
}

void ilG_texture_fromasset(ilG_texture* self, unsigned unit, const ilA_file *iface, il_base *file)
{
    size_t size;
    void *contents = ilA_contents(iface, file, &size);
    ilG_texture_fromimage(self, unit, ilA_img_load(contents, size));
}

void ilG_texture_fromGL(ilG_texture* self, unsigned unit, GLenum target, GLuint tex)
{
    self->units[unit] = (ilG_textureunit) {
        .used = 1,
        .tex = tex,
        .mode = target
    };
}

static GLenum getImgFormat(const ilA_img *img)
{
    switch (img->channels) {
        case ILA_IMG_RGBA:  return GL_RGBA;
        case ILA_IMG_RGB:   return GL_RGB;
        case ILA_IMG_RG:    return GL_RG;
        case ILA_IMG_R:     return GL_RED;
        default: il_error("Unhandled colour format"); return 0;
    }
}

static GLenum getImgType(const ilA_img *img)
{
    if (img->fp) {
        return GL_FLOAT;
    }
    switch (img->depth) {
        case 8:     return GL_UNSIGNED_BYTE;
        case 16:    return GL_UNSIGNED_SHORT;
        case 32:    return GL_UNSIGNED_INT;
        default: il_error("Unhandled bit depth"); return 0;
    }
}

static GLenum getImgIFormat(const ilA_img *img)
{
    unsigned i;
    static const struct {
        enum ilA_imgchannels channels;
        unsigned depth;
        unsigned fp;
        GLenum format;
    } mapping_table[] = {
        {ILA_IMG_R,     8,  0,  GL_R8},
        {ILA_IMG_R,     16, 0,  GL_R16},
        {ILA_IMG_R,     32, 1,  GL_R32F},

        {ILA_IMG_RG,    8,  0,  GL_RG8},
        {ILA_IMG_RG,    16, 0,  GL_RG16},
        {ILA_IMG_RG,    32, 1,  GL_RG32F},

        {ILA_IMG_RGB,   8,  0, GL_RGB8},
        {ILA_IMG_RGB,   16, 0, GL_RGB16},
        {ILA_IMG_RGB,   32, 1, GL_RGB32F},

        {ILA_IMG_RGBA,  8,  0, GL_RGBA8},
        {ILA_IMG_RGBA,  16, 0, GL_RGBA16},
        {ILA_IMG_RGBA,  32, 1, GL_RGBA32F},

        {0, 0, 0, 0}
    };

    for (i = 0; mapping_table[i].depth; i++) {
        if (mapping_table[i].channels == img->channels && mapping_table[i].fp == img->fp && mapping_table[i].depth == img->depth) {
            return mapping_table[i].format;
        }
    }
    
    return getImgFormat(img);
}

void ilG_texture_fromimage(ilG_texture *self, unsigned unit, ilA_img *img)
{
    GLenum format, internalformat, type;

    format = getImgFormat(img);
    type = getImgType(img);
    internalformat = getImgIFormat(img);
    ilG_texture_fromdata(self, unit, GL_TEXTURE_2D, internalformat, img->width, img->height, 1, format, type, img->data);
}

void ilG_texture_cubemap(ilG_texture *self, unsigned unit, struct ilA_img *faces[6])
{
    unsigned i;
    static const GLenum targets[6] = {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
    };
    GLuint tex;

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
    for (i = 0; i < 6; i++) {
        glTexImage2D(targets[i], 0, getImgIFormat(faces[i]), faces[i]->width, 
                     faces[i]->height, 0, getImgFormat(faces[i]), 
                     getImgType(faces[i]), faces[i]->data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    ilG_texture_fromGL(self, unit, GL_TEXTURE_CUBE_MAP, tex);
}

void ilG_texture_fromdata(ilG_texture* self, unsigned unit, GLenum target, 
    GLenum internalformat, unsigned width, unsigned height, unsigned depth, GLenum format, 
    GLenum type, void *data)
{
    GLuint tex;

    glGenTextures(1, &tex);
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
    //.hh = {0},
    .bind = NULL,
    .action = texture_update,
    .unbind = texture_unbind
};

void ilG_texture_init()
{
    il_impl(&ilG_texture_type, &texture_bindable);
}

