#include "context.h"

ilG_context* ilG_context_new()
{
    ilG_context* ctx = calloc(sizeof(ilG_context), 1);

    GLint num_texunits;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &num_texunits);
    ctx->texunits = calloc(sizeof(unsigned), num_texunits);
    ctx->num_texunits = num_texunits;

    return ctx;
}

