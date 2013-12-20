#include "framebuffer.h"

#include "util/assert.h"
#include "graphics/context.h"
#include "util/log.h"

struct target {
    GLenum target, fmt, attach;
    unsigned awidth, aheight;
    float rwidth, rheight;
    char name[64];
};

struct ilG_fbo {
    GLuint fbo;
    GLuint *textures;
    struct target *targets;
    unsigned num, complete;
    char name[64];
};

ilG_fbo *ilG_fbo_new()
{
    ilG_fbo *fbo = calloc(1, sizeof(ilG_fbo));
    glGenFramebuffers(1, &fbo->fbo);
    return fbo;
}

void ilG_fbo_name(ilG_fbo *self, unsigned target, const char *name)
{
    il_return_on_fail(strlen(name) < 63);
    if (target == ilG_fbo_self) { 
        strcpy(self->name, name);
        return;
    }
    il_return_on_fail(target < self->num);
    strcpy(self->targets[target].name, name);
}

void ilG_fbo_numTargets(ilG_fbo *self, unsigned num)
{
    if (self->textures) {
        glDeleteTextures(self->num, self->textures);
    }
    self->textures = realloc(self->textures, num * sizeof(GLuint));
    glGenTextures(num, self->textures);
    self->targets = realloc(self->targets, num * sizeof(struct target));
    memset(self->targets, 0, num * sizeof(struct target));
    self->num = num;
}

void ilG_fbo_size_abs(ilG_fbo *self, unsigned target, unsigned w, unsigned h)
{
    il_return_on_fail(target < self->num);
    self->targets[target].awidth = w;
    self->targets[target].aheight = h;
}

void ilG_fbo_size_rel(ilG_fbo *self, unsigned target, float w, float h)
{
    il_return_on_fail(target < self->num);
    self->targets[target].rwidth = w;
    self->targets[target].rheight = h;
}

void ilG_fbo_texture(ilG_fbo *self, unsigned target, GLenum type, GLenum fmt, GLenum attachment)
{
    il_return_on_fail(target < self->num);
    self->targets[target].target = type;
    self->targets[target].fmt = fmt;
    self->targets[target].attach = attachment;
}

GLuint ilG_fbo_getTex(ilG_fbo *self, unsigned target)
{
    il_return_val_on_fail(target < self->num, 0);
    return self->textures[target];
}

const char *ilG_fbo_getName(ilG_fbo *self, unsigned target)
{
    if (target == ilG_fbo_self) {
        return self->name;
    }
    il_return_null_on_fail(target < self->num);
    return self->targets[target].name;
}

void ilG_fbo_bind(ilG_fbo *self, enum ilG_fbo_type type)
{
    GLenum drawbufs[self->num], target;
    unsigned i;
    for (i = 0; i < self->num; i++) {
        drawbufs[i] = self->targets[i].attach;
    }
    if (type == (ILG_FBO_READ|ILG_FBO_WRITE)) {
        target = GL_FRAMEBUFFER;
    } else if (type == ILG_FBO_READ) {
        target = GL_READ_FRAMEBUFFER;
    } else if (type == ILG_FBO_WRITE) {
        target = GL_DRAW_FRAMEBUFFER;
    } else {
        il_error("Invalid enum value");
        return;
    }
    glBindFramebuffer(target, self->fbo);
    if (type == ILG_FBO_READ) {
        glReadBuffer(drawbufs[0]);
    } else {
        glDrawBuffers(self->num, drawbufs);
    }
}

void ilG_fbo_bind_with(ilG_fbo *self, enum ilG_fbo_type type,  unsigned n, unsigned *order)
{
    GLenum drawbufs[n], target;
    unsigned i;
    for (i = 0; i < n; i++) {
        unsigned id = order[i];
        il_return_on_fail(id < self->num);
        drawbufs[i] = self->targets[id].attach;
    }
    if (type == (ILG_FBO_READ|ILG_FBO_WRITE)) {
        target = GL_FRAMEBUFFER;
    } else if (type == ILG_FBO_READ) {
        target = GL_READ_FRAMEBUFFER;
    } else if (type == ILG_FBO_WRITE) {
        target = GL_DRAW_FRAMEBUFFER;
    } else {
        il_error("Invalid enum value");
        return;
    }
    glBindFramebuffer(target, self->fbo);
    if (type == ILG_FBO_READ) {
        glReadBuffer(drawbufs[0]);
    } else {
        glDrawBuffers(n, drawbufs);
    }

}

void ilG_fbo_swap(ilG_fbo *self, unsigned target1, unsigned target2)
{
    il_return_on_fail(target1 < self->num && target2 < self->num);
    struct target temp = self->targets[target1];
    GLuint tex = self->textures[target1];
    self->targets[target1] = self->targets[target2];
    self->textures[target1] = self->textures[target2];
    self->targets[target2] = temp;
    self->textures[target2] = tex;
}

int /*failure*/ ilG_fbo_build(ilG_fbo *self, ilG_context *ctx)
{
    il_return_val_on_fail(ctx, 1);
    il_log("Building framebuffer \"%s\"", self->name);
    unsigned i;
    glBindFramebuffer(GL_FRAMEBUFFER, self->fbo);
    for (i = 0; i < self->num; i++) {
        struct target *t = self->targets + i;
        glBindTexture(t->target, self->textures[i]);
        glTexImage2D(t->target, 0, t->fmt, t->awidth + t->rwidth*ctx->width, t->aheight + t->rheight*ctx->height, 0, t->fmt, GL_UNSIGNED_BYTE, NULL);
        glFramebufferTexture2D(GL_FRAMEBUFFER, t->attach, t->target, self->textures[i], 0);
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        const char *status_str;
        switch(status) {
            case GL_FRAMEBUFFER_UNDEFINED:                      status_str = "GL_FRAMEBUFFER_UNDEFINED";                        break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:          status_str = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";            break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:  status_str = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";    break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:         status_str = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";           break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:         status_str = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";           break;
            case GL_FRAMEBUFFER_UNSUPPORTED:                    status_str = "GL_FRAMEBUFFER_UNSUPPORTED";                      break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:         status_str = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";           break;
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:       status_str = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";         break;
            default:                                            status_str = "???";                                             break;
        }
        il_error("Unable to create framebuffer for context: %s", status_str);
        return 1;
    }
    self->complete = 1;
    return 0;
}

