#include "helper.h"

#include "asset/image.h"
#include "graphics/tex.h"
#include "util/log.h"

extern ilA_fs demo_fs;

helper_result helper_create(helper_config *conf)
{
    helper_result r;
    r.context = ilG_context_new();
    ilG_renderman *rm = &r.context->manager;
    if (conf->name[0]) {
        r.context->initialTitle = strdup(conf->name);
        //ilG_context_rename(r.context, conf->name);
    }
    ilG_context_hint(r.context, ILG_CONTEXT_DEBUG_RENDER, 1);
    ilG_context_hint(r.context, ILG_CONTEXT_VSYNC, 1);
    if (conf->geom) {
        r.geom = ilG_build(ilG_geometry_builder(r.context), rm);
        ilG_handle_addRenderer(r.context->root, r.geom);
        if (conf->sky) {
            ilA_img imgs[6];
            for (unsigned i = 0; i < 6; i++) {
                ilA_imgerr res = ilA_img_loadfile(&imgs[i], &demo_fs, conf->skytex[i]);
                if (res) {
                    il_error("%s", ilA_img_strerror(res));
                    goto sky_fail;
                }
            }
            ilG_tex tex;
            ilG_tex_loadcube(&tex, imgs);
            r.sky = ilG_build(ilG_skybox_builder(tex, r.context), rm);
            ilG_handle_addRenderer(r.geom, r.sky);
          sky_fail:;
        }
    }
    if (conf->lights) {
        r.ambient_col = calloc(1, sizeof(il_vec3));
        *r.ambient_col = il_vec3_new(1,1,1);
        r.ambient = ilG_build(ilG_ambient_builder(r.context, r.ambient_col), rm);
        ilG_handle_addRenderer(r.context->root, r.ambient);
        r.lights = ilG_build(ilG_pointlight_builder(r.context), rm);
        ilG_handle_addRenderer(r.context->root, r.lights);
    }
    if (conf->out) {
        r.out = ilG_build(ilG_out_builder(r.context, NULL, NULL), rm);
        ilG_handle_addRenderer(r.context->root, r.out);
    }
    return r;
}
