#ifndef ILG_TEXTURE_H
#define ILG_TEXTURE_H

#include <GL/glew.h>

struct il_positionable;
struct ilG_texture;
struct ilG_context;

typedef void (*ilG_texture_bind_cb)(struct ilG_context*, void*);
typedef void (*ilG_texture_update_cb)(struct ilG_context*, struct il_positionable*, void*);

typedef struct ilG_texture {
    unsigned int id;
    const char *name;
    ilG_texture_bind_cb bind, unbind;
    ilG_texture_update_cb update;
    void *bind_ctx, *update_ctx, *unbind_ctx;
} ilG_texture;

ilG_texture *ilG_texture_default;

struct ilA_asset;

ilG_texture* ilG_texture_new();
void ilG_texture_setName(ilG_texture* self, const char *name);
void ilG_texture_fromfile(ilG_texture* self, unsigned unit, const char *name);
void ilG_texture_fromasset(ilG_texture* self, unsigned unit, struct ilA_asset* asset);
void ilG_texture_fromGL(ilG_texture* self, unsigned unit, GLenum target, GLuint tex);
void ilG_texture_fromdata(ilG_texture* self, unsigned unit, GLenum target, 
    GLenum internalformat, unsigned width, unsigned height, unsigned depth,
    GLenum format, GLenum type, void *data);
GLuint ilG_texture_getTex(ilG_texture* self, unsigned unit, GLenum *target);
void ilG_texture_setFilter(ilG_texture* self, unsigned unit, GLenum min_filter, GLenum mag_filter);

ilG_texture* ilG_texture_fromId(unsigned int id); // tracker.c
void ilG_texture_assignId(ilG_texture*);
void ilG_texture_setId(ilG_texture*, unsigned int id);

#endif

