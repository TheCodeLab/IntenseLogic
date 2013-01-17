#include "texture.h"

#include <stdlib.h>
#include <GL/glew.h>

#include "asset/asset.h"
#include "asset/texture.h"
#include "graphics/tracker.h"
#include "graphics/context.h"
#include "graphics/textureunit.h"

struct GLtexture {
    ilG_texture parent;
    GLuint object;
};

static void update(ilG_context* context, struct il_positionable* pos, void * ctx)
{
    (void)ctx, (void)pos;
    unsigned int i;

    static struct ilG_material* mtl = NULL;

    if (mtl == context->material) return;
    mtl = context->material;

    struct GLtexture *tex = (struct GLtexture*)(context->texture);
    for (i = 0; i < context->num_active; i++) {
        if (context->texunits[i] == ILG_TUNIT_COLOR0) {
            glActiveTexture(i);
            glBindTexture(GL_TEXTURE_2D, tex->object);
        }
    }
}

ilG_texture* ilG_texture_fromfile(const char *name)
{
    ilA_asset* asset = ilA_open(il_fromC(name));
    return ilG_texture_fromasset(asset);
}

ilG_texture* ilG_texture_fromasset(ilA_asset* asset)
{
    struct GLtexture *tex = calloc(1, sizeof(struct GLtexture));

    tex->object = ilA_assetToTexture(asset);
    tex->parent.update = &update;
    tex->parent.name = il_StoC(ilA_getPath(asset));

    return &tex->parent;
}

static struct GLtexture def;

void ilG_texture_init()
{
    // 1x1 white texture for now I guess
    static const unsigned char data[] = {
        255, 255, 255, 255
    };

    def.parent.name = "Default Texture";

    glGenTextures(1, &def.object);
    glBindTexture(GL_TEXTURE_2D, def.object);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, 
        GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    def.parent.update = &update;

    ilG_texture_default = &def.parent;
    ilG_texture_assignId(ilG_texture_default);
}

