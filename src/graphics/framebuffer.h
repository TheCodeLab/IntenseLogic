#ifndef ILG_FRAMEBUFFER_H
#define ILG_FRAMEBUFFER_H

#include <GL/glew.h>

#define ilG_fbo_self ((unsigned)-1)

struct ilG_context;

typedef struct ilG_fbo ilG_fbo;

enum ilG_fbo_type {
    ILG_FBO_READ    = 1,
    ILG_FBO_WRITE   = 2,
    ILG_FBO_RW      = ILG_FBO_READ|ILG_FBO_WRITE,
};

ilG_fbo *ilG_fbo_new();
void ilG_fbo_free(ilG_fbo *self);

void ilG_fbo_numTargets(ilG_fbo *self, unsigned num);
void ilG_fbo_name(ilG_fbo *self, unsigned target, const char *name);
void ilG_fbo_size_abs(ilG_fbo *self, unsigned target, unsigned w, unsigned h);
void ilG_fbo_size_rel(ilG_fbo *self, unsigned target, float w, float h);
void ilG_fbo_texture(ilG_fbo *self, unsigned target, GLenum type, GLenum fmt, GLenum attachment);
GLuint ilG_fbo_getTex(ilG_fbo *self, unsigned target);
const char *ilG_fbo_getName(ilG_fbo *self, unsigned target);
void ilG_fbo_bind(ilG_fbo *self, enum ilG_fbo_type type);
void ilG_fbo_swap(ilG_fbo *self, unsigned target1, unsigned target2);
void ilG_fbo_bind_with(ilG_fbo *self, enum ilG_fbo_type type, unsigned n, unsigned *order);

int /*failure*/ ilG_fbo_build(ilG_fbo *self, struct ilG_context *ctx);

#endif

