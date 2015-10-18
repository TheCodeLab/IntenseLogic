#ifndef ILG_TEX_H
#define ILG_TEX_H

#include <stdbool.h>

#include "tgl/gl.h"
#include "asset/image.h"

typedef struct ilG_tex {
    GLenum target;
    GLenum object;
} ilG_tex;

void ilG_tex_free(ilG_tex *tex);
ilA_imgerr ilG_tex_loadfile(ilG_tex *tex, ilA_fs *fs, const char *file);
void ilG_tex_loadcube(ilG_tex *tex, ilA_img faces[6]);
void ilG_tex_loadimage(ilG_tex *tex, ilA_img img);
void ilG_tex_loaddata(ilG_tex *tex, GLenum target, GLenum internalformat,
                      unsigned width, unsigned height, unsigned depth,
                      GLenum format, GLenum type, void *data);
void ilG_tex_bind(ilG_tex *tex, unsigned unit);

#endif
