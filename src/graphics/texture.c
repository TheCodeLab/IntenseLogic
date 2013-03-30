#include "texture.h"

#include <stdlib.h>
#include <GL/glew.h>

#include "asset/asset.h"
#include "asset/texture.h"
#include "graphics/tracker.h"
#include "graphics/context.h"
#include "graphics/textureunit.h"
#include "util/log.h"

struct texture_ctx {
    struct ilG_material* mtl;
    struct texture_unit {
        int used;
        GLuint tex;
        GLenum mode;
    } units[ILG_TUNIT_NUMUNITS];
};

static void texture_update(ilG_context* context, struct il_positionable* pos, void *user)
{
    (void)pos;
    unsigned int i;
    struct texture_ctx *ctx = user;

    if (ctx->mtl == context->material) return;
    ctx->mtl = context->material;
    for (i = 0; i < context->num_active; i++) {
        if (ctx->units[context->texunits[i]].used) {
            glActiveTexture(i);
            glBindTexture(ctx->units[context->texunits[i]].mode, ctx->units[context->texunits[i]].tex);
        }
    }
}

ilG_texture* ilG_texture_new()
{
    ilG_texture *tex = calloc(1, sizeof(ilG_texture));
    struct texture_ctx *ctx = calloc(1, sizeof(struct texture_ctx));
    tex->update = &texture_update;
    tex->bind_ctx = 
    tex->update_ctx = 
    tex->unbind_ctx = ctx;
    tex->name = "Unnamed";
    ilG_texture_assignId(tex);
    return tex;
}

char *strdup(const char*);
void ilG_texture_setName(ilG_texture* self, const char *name)
{
    self->name = strdup(name);
}

void ilG_texture_fromfile(ilG_texture* self, unsigned unit, const char *name)
{
    ilA_asset* asset = ilA_open(il_string_new(name, strlen(name)));
    ilG_texture_fromasset(self, unit, asset);
}

void ilG_texture_fromasset(ilG_texture* self, unsigned unit, ilA_asset* asset)
{
    struct texture_ctx *ctx = self->update_ctx;
    ctx->units[unit] = (struct texture_unit) {
        .used = 1,
        .tex = ilA_assetToTexture(asset),
        .mode = GL_TEXTURE_2D
    };
}

void ilG_texture_fromGL(ilG_texture* self, unsigned unit, GLenum target, GLuint tex)
{
    struct texture_ctx *ctx = self->update_ctx;

    ctx->units[unit] = (struct texture_unit) {
        .used = 1,
        .tex = tex,
        .mode = target
    };
}

void ilG_texture_fromdata(ilG_texture* self, unsigned unit, GLenum target, 
    GLenum internalformat, unsigned width, unsigned height, unsigned depth, GLenum format, 
    GLenum type, void *data)
{
    struct texture_ctx *ctx = self->update_ctx;
    GLuint tex;

    glGenTextures(target, &tex);
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

    ctx->units[unit] = (struct texture_unit) {
        .used = 1,
        .tex = tex,
        .mode = target
    };
}

GLuint ilG_texture_getTex(ilG_texture* self, unsigned unit, GLenum *target)
{
    struct texture_ctx *ctx = self->update_ctx;

    if (target) {
        *target = ctx->units[unit].mode;
    }
    return ctx->units[unit].tex;
}

void ilG_texture_setFilter(ilG_texture* self, unsigned unit, GLenum min_filter, GLenum mag_filter)
{
    struct texture_ctx *ctx = self->update_ctx;

    glBindTexture(ctx->units[unit].mode, ctx->units[unit].tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
}

static ilG_texture def;
static struct texture_ctx defctx;

void ilG_texture_init()
{
    // 1x1 white texture for now I guess
    static const unsigned char data[] = {
        255, 255, 255, 255
    };

    def.name = "Default Texture";
    glGenTextures(1, &defctx.units[ILG_TUNIT_COLOR0].tex);
    glBindTexture(GL_TEXTURE_2D, defctx.units[ILG_TUNIT_COLOR0].tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, 
        GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    def.update = &texture_update;
    def.bind_ctx =
    def.update_ctx =
    def.unbind_ctx = &defctx;
    ilG_texture_default = &def;
    ilG_texture_assignId(ilG_texture_default);
}

