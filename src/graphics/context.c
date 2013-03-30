#include "context.h"

#include <stdlib.h>

#include "util/log.h"
#include "graphics/glutil.h"

ilG_context* ilG_context_new(int w, int h)
{
    ilG_context* ctx = calloc(sizeof(ilG_context), 1);
    ctx->width = w;
    ctx->height = h;
    GLint num_texunits;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &num_texunits);
    ctx->texunits = calloc(sizeof(unsigned), num_texunits);
    ctx->num_texunits = num_texunits;
    ilG_testError("Unknown error");
    glGenFramebuffers(1, &ctx->framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, ctx->framebuffer);
    glGenTextures(5, &ctx->fbtextures[0]);
    ilG_testError("Unable to generate framebuffer");
    // depth texture
    glBindTexture(GL_TEXTURE_RECTANGLE, ctx->fbtextures[0]); 
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_RECTANGLE, ctx->fbtextures[0], 0);
    ilG_testError("Unable to create depth buffer");
    // accumulation
    glBindTexture(GL_TEXTURE_RECTANGLE, ctx->fbtextures[1]);
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, ctx->fbtextures[1], 0);
    ilG_testError("Unable to create accumulation buffer");
    // normal
    glBindTexture(GL_TEXTURE_RECTANGLE, ctx->fbtextures[2]); 
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB, w, h, 0, GL_RGB, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_RECTANGLE, ctx->fbtextures[2], 0);
    ilG_testError("Unable to create normal buffer");
    // diffuse
    glBindTexture(GL_TEXTURE_RECTANGLE, ctx->fbtextures[3]); 
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_RECTANGLE, ctx->fbtextures[3], 0);
    ilG_testError("Unable to create diffuse buffer");
    // specular
    glBindTexture(GL_TEXTURE_RECTANGLE, ctx->fbtextures[4]); 
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_RECTANGLE, ctx->fbtextures[4], 0);
    ilG_testError("Unable to create specular buffer");
    // completeness testing
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
        il_fatal("Unable to create framebuffer for context: %s", status_str);
    }

    return ctx;
}

