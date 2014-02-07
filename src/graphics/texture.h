#ifndef ILG_TEXTURE_H
#define ILG_TEXTURE_H

#include <GL/glew.h>

#include "common/base.h"
#include "graphics/textureunit.h"

struct il_positionable;
struct ilG_texture;
struct ilG_context;

typedef struct ilG_textureunit {
    int used;
    GLuint tex;
    GLenum mode;
} ilG_textureunit;

typedef struct ilG_texture {
    il_base base;
    char *name;
    ilG_textureunit units[ILG_TUNIT_NUMUNITS];
    struct ilG_context *context;
    struct ilG_material *last_mtl;
} ilG_texture;

extern il_type ilG_texture_type;

struct ilA_file;
struct il_base;
struct ilA_img;

//ilG_texture* ilG_texture_new();
#define ilG_texture_new() il_new(&ilG_texture_type)
void ilG_texture_setContext(ilG_texture* self, struct ilG_context *context);
void ilG_texture_setName(ilG_texture* self, const char *name);
void ilG_texture_fromfile(ilG_texture* self, unsigned unit, const char *name);
void ilG_texture_fromasset(ilG_texture* self, unsigned unit, const struct ilA_file* iface, struct il_base *file);
void ilG_texture_fromGL(ilG_texture* self, unsigned unit, GLenum target, GLuint tex);
void ilG_texture_fromimage(ilG_texture *self, unsigned unit, struct ilA_img *img);
void ilG_texture_cubemap(ilG_texture *self, unsigned unit, struct ilA_img *faces[6]);
void ilG_texture_fromdata(ilG_texture* self, unsigned unit, GLenum target, 
    GLenum internalformat, unsigned width, unsigned height, unsigned depth,
    GLenum format, GLenum type, void *data);
GLuint ilG_texture_getTex(ilG_texture* self, unsigned unit, GLenum *target);
void ilG_texture_setFilter(ilG_texture* self, unsigned unit, GLenum min_filter, GLenum mag_filter);
GLuint ilG_texture_getRaw(ilG_texture *self, unsigned unit);

#endif

