#include "texture.h"

#include <stdlib.h>
#include <IL/il.h>
#include <IL/ilu.h>

#include "asset/asset.h"
#include "graphics/tracker.h"

struct GLtexture {
    ilG_texture parent;
    GLuint object;
};

static void bind(ilG_texture* rawtex, void * ctx)
{
    (void)ctx;
    struct GLtexture *tex = (struct GLtexture*)rawtex;
    glBindTexture(GL_TEXTURE_2D, tex->object);
}

ilG_texture* ilG_texture_fromfile(const char *name)
{
    ilA_asset* asset = ilA_open(il_fromC(name));
    return ilG_texture_fromasset(asset);
}

ilG_texture* ilG_texture_fromasset(ilA_asset* asset)
{
    struct GLtexture *tex = calloc(1, sizeof(struct GLtexture));

    //tex->tex = ilutGLLoadImage(il_toC(ilA_getPath(asset)));
    tex->parent.bind = &bind;

    return &tex->parent;
}

struct GLtexture def;

void ilG_texture_init()
{
    // 1x1 white texture for now I guess
    const static unsigned char data[] = {
        255, 255, 255, 255
    };

    def.parent.name = "Default Texture";

    glGenTextures(1, &def.object);
    glBindTexture(GL_TEXTURE_2D, def.object);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGB, 
        GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    ilG_texture_default = &def.parent;
    ilG_texture_assignId(ilG_texture_default);
}

