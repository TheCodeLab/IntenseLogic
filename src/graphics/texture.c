#include "texture.h"

#include <stdlib.h>
#include <IL/il.h>
#include <IL/ilu.h>

#include "asset/asset.h"

struct GLtexture {
    ilG_texture parent;
    GLuint tex;
};

static void bind(ilG_texture* rawtex, void * ctx)
{
    (void)ctx;
    struct GLtexture *tex = (struct GLtexture*)rawtex;
    glBindTexture(GL_TEXTURE_2D, tex->tex);
}

ilG_texture* ilG_texture_fromfile(const char *name)
{
    ilA_asset* asset = ilA_open(il_fromC(name));
    return ilG_texture_fromasset(asset);
}

ilG_texture* ilG_texture_fromasset(ilA_asset* asset)
{
    struct GLtexture *tex = calloc(1, sizeof(struct GLtexture));

    tex->tex = ilutGLLoadImage(il_toC(ilA_getPath(asset)));
    tex->parent.bind = &bind;

    return tex;
}

