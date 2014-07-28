#include "helper.h"

#include "asset/image.h"
#include "graphics/skyboxpass.h"
#include "graphics/geometrypass.h"
#include "graphics/lightpass.h"
#include "graphics/outpass.h"
#include "graphics/tex.h"

helper_result helper_create(helper_config *conf)
{
    helper_result r;
    r.context = ilG_context_new();
    if (conf->name[0]) {
        r.context->initialTitle = strdup(conf->name);
        //ilG_context_rename(r.context, conf->name);
    }
    ilG_context_hint(r.context, ILG_CONTEXT_DEBUG_RENDER, 1);
    ilG_context_hint(r.context, ILG_CONTEXT_VSYNC, 1);
    if (conf->sky) {
        ilA_img *imgs[6];
        for (unsigned i = 0; i < 6; i++) {
            imgs[i] = ilA_img_loadfile(conf->skytex[i]);
        }
        ilG_tex tex;
        ilG_tex_loadcube(&tex, imgs);
        r.sky = ilG_build(ilG_skybox_builder(tex), r.context);
        ilG_handle_addRenderer(r.context->root, r.sky);
    }
    if (conf->geom) {
        r.geom = ilG_build(ilG_geometry_builder(), r.context);
        ilG_handle_addRenderer(r.context->root, r.geom);
    }
    if (conf->lights) {
        r.lights = ilG_build(ilG_pointlight_builder(), r.context);
        ilG_handle_addRenderer(r.context->root, r.lights);
    }
    if (conf->out) {
        r.out = ilG_build(ilG_out_builder(), r.context);
        ilG_handle_addRenderer(r.context->root, r.out);
    }
    return r;
}
