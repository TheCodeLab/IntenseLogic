#ifndef ILG_RENDERER_H
#define ILG_RENDERER_H

#include "common/world.h"

struct ilG_material;
struct ilG_drawable3d;
struct ilG_context;
struct ilG_renderer;
struct ilG_tex;
struct il_mat;

#define il_pair(name, fst, snd) typedef struct name {fst first; snd second;} name

typedef void (*ilG_message_fn)(void *obj, int type, il_value *v);
typedef struct ilG_msgsink {
    ilG_message_fn fn;
    unsigned id;
} ilG_msgsink;

typedef unsigned ilG_rendid;
typedef struct ilG_renderer ilG_renderer;

typedef void (*ilG_free_fn)(void *);
typedef void (*ilG_update_fn)(void *obj, ilG_rendid id);
typedef void (*ilG_draw_fn)(void *obj, ilG_rendid id, struct il_mat **mats, unsigned num_mats);
typedef struct ilG_buildresult {
    const ilG_free_fn free;
    const ilG_update_fn update;
    const ilG_draw_fn draw;
    int *types;
    unsigned num_types;
    void *obj;
} ilG_buildresult;

typedef bool (*ilG_build_fn)(void *obj, ilG_rendid id, struct ilG_context *context, ilG_buildresult *out);
typedef struct ilG_builder {
    const ilG_build_fn build;
    void *obj;
} ilG_builder;

typedef struct ilG_handle {
    ilG_rendid id;
    struct ilG_context *context;
} ilG_handle;

typedef struct ilG_coordsys {
    const ilG_free_fn free;
    void (* const buildmats)(void *, struct il_mat *out, int type, size_t n);
    void *obj;
} ilG_coordsys;

typedef struct ilG_light {
    unsigned id;
    il_vec3 color;
    float radius;
} ilG_light;

typedef struct ilG_objrenderer {
    const ilG_draw_fn draw;
    unsigned coordsys;
    int *types;
    unsigned num_types;
} ilG_objrenderer;

struct ilG_renderer {
    const ilG_free_fn free;
    const ilG_update_fn update;
    IL_ARRAY(unsigned,) rchildren;
    IL_ARRAY(unsigned,) mchildren;
    IL_ARRAY(ilG_light,) lights;
    void *obj;
};

il_pair(ilG_multirenderer, ilG_renderer, ilG_objrenderer);
il_pair(ilG_rendstorage,ilG_rendid, il_table);
il_pair(ilG_rendname,   ilG_rendid, unsigned);

typedef struct ilG_rendermanager {
    IL_ARRAY(ilG_renderer,)     renderers;
    IL_ARRAY(ilG_rendid,)       rendids;
    IL_ARRAY(ilG_multirenderer,)multirenderers;
    IL_ARRAY(ilG_rendid,)       multirendids;
    IL_ARRAY(ilG_msgsink,)      sinks;
    IL_ARRAY(ilG_rendstorage,)  storages;
    IL_ARRAY(ilG_rendname,)     namelinks;
    IL_ARRAY(char*,)            names;
    IL_ARRAY(ilG_coordsys,)     coordsystems;
    unsigned curid;
} ilG_rendermanager;

typedef struct ilG_legacy ilG_legacy;
#define ilG_legacy_builder(p) ilG_builder_wrap(p, ilG_legacy_build)
ilG_legacy *ilG_legacy_new(struct ilG_drawable3d *dr, struct ilG_material *mtl);
void ilG_legacy_addTexture(ilG_legacy *self, struct ilG_tex tex);
bool ilG_legacy_build(void *ptr, ilG_rendid id, struct ilG_context *context, ilG_buildresult *out);

ilG_builder ilG_builder_wrap(void *obj, const ilG_build_fn build);

ilG_handle ilG_build(ilG_builder self, struct ilG_context *context);
void ilG_handle_destroy(ilG_handle self);
bool ilG_handle_ready(ilG_handle self);
il_table *ilG_handle_storage(ilG_handle self);
const char *ilG_handle_getName(ilG_handle self);
void ilG_handle_addPositionable(ilG_handle self, il_positionable pos);
void ilG_handle_delPositionable(ilG_handle self, il_positionable pos);
void ilG_handle_addRenderer(ilG_handle self, ilG_handle node);
void ilG_handle_delRenderer(ilG_handle self, ilG_handle node);
void ilG_handle_addLight(ilG_handle self, ilG_light light);
void ilG_handle_delLight(ilG_handle self, ilG_light light);
void ilG_handle_message(ilG_handle self, int type, il_value v);

#endif
