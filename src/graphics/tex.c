#include "tex.h"

#include "util/log.h"

void ilG_tex_free(ilG_tex *tex)
{
    glDeleteTextures(1, &tex->object);
}

ilA_imgerr ilG_tex_loadfile(ilG_tex *tex, ilA_fs *fs, const char *file)
{
    ilA_img img;
    ilA_imgerr res = ilA_img_loadfile(&img, fs, file);
    if (res) {
        return res;
    }
    ilG_tex_loadimage(tex, img);
    return ILA_IMG_SUCCESS;
}

static GLenum getImgFormat(const ilA_img *img)
{
    switch (img->channels) {
    case ILA_IMG_RGBA:  return GL_RGBA;
    case ILA_IMG_RGB:   return GL_RGB;
    case ILA_IMG_RG:    return GL_RG;
    case ILA_IMG_R:     return GL_RED;
    }
    il_error("Unhandled colour format");
    return 0;
}

static GLenum getImgType(const ilA_img *img)
{
    switch (img->format) {
    case ILA_IMG_U8:     return GL_UNSIGNED_BYTE;
    case ILA_IMG_U16:    return GL_UNSIGNED_SHORT;
    case ILA_IMG_F32:    return GL_FLOAT;
    }
    il_error("Unhandled image format");
    return 0;
}

static GLenum getImgIFormat(const ilA_img *img)
{
    unsigned i;
    static const struct {
        ilA_imgchannel chans;
        ilA_imgformat fmt;
        GLenum format;
    } mapping_table[] = {
        {ILA_IMG_R,    ILA_IMG_U8,  GL_R8},
        {ILA_IMG_R,    ILA_IMG_U16, GL_R16},
        {ILA_IMG_R,    ILA_IMG_F32, GL_R32F},

        {ILA_IMG_RG,   ILA_IMG_U8,  GL_RG8},
        {ILA_IMG_RG,   ILA_IMG_U16, GL_RG16},
        {ILA_IMG_RG,   ILA_IMG_F32, GL_RG32F},

        {ILA_IMG_RGB,  ILA_IMG_U8,  GL_RGB8},
        {ILA_IMG_RGB,  ILA_IMG_U16, GL_RGB16},
        {ILA_IMG_RGB,  ILA_IMG_F32, GL_RGB32F},

        {ILA_IMG_RGBA, ILA_IMG_U8,  GL_RGBA8},
        {ILA_IMG_RGBA, ILA_IMG_U16, GL_RGBA16},
        {ILA_IMG_RGBA, ILA_IMG_F32, GL_RGBA32F},

        {0, 0, 0}
    };

    for (i = 0; mapping_table[i].chans; i++) {
        if (mapping_table[i].chans == img->channels && mapping_table[i].fmt == img->format) {
            return mapping_table[i].format;
        }
    }

    return getImgFormat(img);
}

void ilG_tex_loadcube(ilG_tex *tex, struct ilA_img faces[6])
{
    static const GLenum targets[6] = {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
    };

    tex->target = GL_TEXTURE_CUBE_MAP;
    glGenTextures(1, &tex->object);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex->object);
    for (unsigned i = 0; i < 6; i++) {
        glTexImage2D(targets[i], 0, getImgIFormat(&faces[i]),
                     faces[i].width, faces[i].height, 0,
                     getImgFormat(&faces[i]),
                     getImgType(&faces[i]), faces[i].data);
        ilA_img_free(faces[i]);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void ilG_tex_loadimage(ilG_tex *tex, struct ilA_img img)
{
    GLenum format, internalformat, type;

    format = getImgFormat(&img);
    type = getImgType(&img);
    internalformat = getImgIFormat(&img);
    ilG_tex_loaddata(tex, GL_TEXTURE_2D, internalformat,
                     img.width, img.height, 1,
                     format, type, img.data);
}

void ilG_tex_loaddata(ilG_tex *tex, GLenum target, GLenum internalformat,
                      unsigned width, unsigned height, unsigned depth,
                      GLenum format, GLenum type, void *data)
{
    tex->target = target;
    glGenTextures(1, &tex->object);
    glBindTexture(target, tex->object);
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
}

void ilG_tex_bind(ilG_tex *tex, unsigned unit)
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(tex->target, tex->object);
}
