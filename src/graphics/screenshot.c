#include "graphics/renderer.h"

#include "asset/image.h"
#include "graphics/context.h"
#include "util/log.h"

ilA_imgerr ilG_screenshot(ilG_context *context, ilA_img *out)
{
    ilA_imgerr res = ilA_img_alloc(out, context->height, context->width, ILA_IMG_U8, ILA_IMG_RGB);
    if (res) {
        return res;
    }
    glBindTexture(GL_TEXTURE_RECTANGLE, tgl_fbo_getTex(&context->accum, 0));
    glGetTexImage(GL_TEXTURE_RECTANGLE, 0, GL_RGB, GL_UNSIGNED_BYTE, out->data);
    return ILA_IMG_SUCCESS;
}
