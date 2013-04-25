#include "heightmap.h"

#include <GL/glew.h>

#include "graphics/drawable3d.h"
#include "common/heightmap.h"
#include "graphics/context.h"
#include "common/positionable.h"
#include "graphics/glutil.h"
#include "graphics/texture.h"
#include "graphics/textureunit.h"
// TODO: heightmaps, compatible with the updated object system
/*
struct height_ctx {
    GLuint vao, vbo, ibo;
    unsigned int vertices;
};

void heightmap_bind(ilG_context* context, void* user)
{
    (void)context;
    ilG_testError("Unknown");
    struct height_ctx* ctx = user;
    glBindVertexArray(ctx->vao);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->ibo);
    glPrimitiveRestartIndex(~0);
    glEnable(GL_PRIMITIVE_RESTART);
    ilG_testError("Failed to bind heightmap");
}

void heightmap_draw(ilG_context* context, il_positionable* pos, void* user)
{
    (void)context, (void)pos;
    struct height_ctx* ctx = user;
    glDrawElements(GL_TRIANGLE_STRIP, ctx->vertices, GL_UNSIGNED_INT, NULL);
    ilG_testError("Failed to draw heightmap");
}

void heightmap_unbind(ilG_context* context, void *user)
{
    (void)context, (void)user;
    glDisable(GL_PRIMITIVE_RESTART);
}

struct ilG_drawable3d* ilG_heightmapDrawable(const il_heightmap* self)
{
    unsigned int width, height;
    il_heightmap_get(self, &width, &height);
    ilG_drawable3d* drawable = calloc(1, sizeof(ilG_drawable3d));
    ilG_drawable3d_assignId(drawable);
    drawable->name = "Heightmap";
    struct height_ctx *ctx = calloc(1, sizeof(struct height_ctx));
    
    glGenVertexArrays(1, &ctx->vao);
    glBindVertexArray(ctx->vao);
    glGenBuffers(1, &ctx->vbo);
    glGenBuffers(1, &ctx->ibo);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo);
    GLuint *vbo = calloc(width * height * 2, sizeof(GLuint));
    unsigned int x, y;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            vbo[(y * width + x)*2 + 0] = x;
            vbo[(y * width + x)*2 + 1] = y;
        }
    }
    glBufferData(GL_ARRAY_BUFFER, width * height * 2 * sizeof(GLuint), vbo, GL_STATIC_DRAW);
    free(vbo);
    glVertexAttribPointer(0, 2, GL_UNSIGNED_INT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->ibo);
    int col = width * 2 + 3,
        size = col * width;
    GLint *ibo = calloc(size, sizeof(GLuint));
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            ibo[(y * col + x) * 2 + 0] = (y * width + 0) + x;
            ibo[(y * col + x) * 2 + 1] = (y * width + 1) + x;
        }
        ibo[y * col + width + 2] = ~0; // primitive restart index
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * sizeof(GLuint), ibo, GL_STATIC_DRAW);
    free(ibo);
    ctx->vertices = size;

    drawable->bind = &heightmap_bind;
    drawable->draw = &heightmap_draw;
    drawable->unbind = &heightmap_unbind;

    drawable->bind_ctx =
    drawable->draw_ctx =
    drawable->unbind_ctx = ctx;

    return drawable;
}

struct texture_ctx {
    GLuint obj;
};

static void tex_update(ilG_context* context, struct il_positionable* pos, void * user)
{
    (void)pos;
    unsigned int i;
    static struct ilG_material* mtl = NULL;
    struct texture_ctx *ctx = user;

    if (mtl == context->material) return;
    mtl = context->material; 
    for (i = 0; i < context->num_active; i++) {
        if (context->texunits[i] == ILG_TUNIT_HEIGHT0) {
            glActiveTexture(i);
            glBindTexture(GL_TEXTURE_2D, ctx->obj);
        }
    }

}

struct ilG_texture* ilG_heightmapTexture(const il_heightmap* self)
{
    (void)self;
    unsigned width, height;
    float *buf = il_heightmap_get(self, &width, &height);
    ilG_texture* tex = calloc(1, sizeof(ilG_texture));
    struct texture_ctx *ctx = calloc(1, sizeof(struct texture_ctx));
    GLuint obj;

    glGenTextures(1, &obj);
    glBindTexture(GL_TEXTURE_2D, obj);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, buf);
    ctx->obj = obj;
    tex->update = tex_update;
    tex->bind_ctx = 
    tex->update_ctx = 
    tex->unbind_ctx = ctx;
    return tex;
}
*/
