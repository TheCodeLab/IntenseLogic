#include "tex.h"

#include "asset/image.h"
#include "util/log.h"

void ilG_tex_loadfile(ilG_tex *self, const char *file)
{
    ilA_img *img = ilA_img_loadfile(file);
    if (!img) {
        return;
    }
    ilG_tex_loadimage(self, img);
}

void ilG_tex_loadasset(ilG_tex *self, const struct ilA_file *iface, void *file)
{
    size_t size;
    void *contents = ilA_contents(iface, file, &size);
    ilG_tex_loadimage(self, ilA_img_load(contents, size));
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

static void tex_cube_build(ilG_tex *self, struct ilG_context *context)
{
    (void)context;
    ilA_img **faces = self->data;
    static const GLenum targets[6] = {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
    };

    glGenTextures(1, &self->object);
    glBindTexture(GL_TEXTURE_CUBE_MAP, self->object);
    for (unsigned i = 0; i < 6; i++) {
        glTexImage2D(targets[i], 0, getImgIFormat(faces[i]), faces[i]->width, 
                     faces[i]->height, 0, getImgFormat(faces[i]), 
                     getImgType(faces[i]), faces[i]->data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void ilG_tex_loadcube(ilG_tex *self, struct ilA_img *faces[6])
{
    ilA_img **ctx = calloc(6, sizeof(ilA_img*));
    memcpy(ctx, faces, sizeof(ilA_img*) * 6);
    self->data = ctx;
    self->build = tex_cube_build;
}

void ilG_tex_loadimage(ilG_tex *self, struct ilA_img *img)
{
    GLenum format, internalformat, type;

    format = getImgFormat(img);
    type = getImgType(img);
    internalformat = getImgIFormat(img);
    ilG_tex_loaddata(self, GL_TEXTURE_2D, internalformat, img->width, img->height, 1, format, type, img->data);
}

struct data_ctx {
    GLenum internal, format, type;
    unsigned width, height, depth;
    void *data;
};
static void tex_data_build(ilG_tex *self, struct ilG_context *context)
{
    (void)context;
    struct data_ctx *ctx = self->data;
    glGenTextures(1, &self->object);
    glBindTexture(self->target, self->object);
    switch (self->target) {
        case GL_TEXTURE_1D:
        case GL_PROXY_TEXTURE_1D:
        glTexImage1D(self->target, 0, ctx->internal, ctx->width, 0, ctx->format, ctx->type, ctx->data);
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
        glTexImage2D(self->target, 0, ctx->internal, ctx->width, ctx->height, 0, ctx->format, ctx->type, ctx->data);
        break;
        case GL_TEXTURE_3D:
        case GL_PROXY_TEXTURE_3D:
        case GL_TEXTURE_2D_ARRAY:
        case GL_PROXY_TEXTURE_2D_ARRAY:
        glTexImage3D(self->target, 0, ctx->internal, ctx->width, ctx->height, ctx->depth, 0, ctx->format, ctx->type, ctx->data);
        break;
        default:
        il_error("Unknown texture format");
        return;
    }
    free(ctx->data);
    free(ctx);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void ilG_tex_loaddata(ilG_tex *self, GLenum target, GLenum internalformat, unsigned width, unsigned height, unsigned depth, GLenum format, GLenum type, void *data)
{
    self->target = target;
    struct data_ctx *ctx = calloc(1, sizeof(struct data_ctx));
    ctx->internal = internalformat;
    ctx->width = width;
    ctx->height = height;
    ctx->depth = depth;
    ctx->format = format;
    ctx->type = type;
    ctx->data = data;
    self->build = tex_data_build;
    self->data = ctx;
}

void ilG_tex_bind(ilG_tex *self)
{
    glActiveTexture(GL_TEXTURE0 + self->unit);
    glBindTexture(self->target, self->object);
}

void ilG_tex_build(ilG_tex *self, struct ilG_context *context)
{
    self->build(self, context);
}

