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
typedef unsigned ilG_cosysid;
typedef struct ilG_renderer ilG_renderer;

typedef void (*ilG_free_fn)(void *);
typedef void (*ilG_update_fn)(void *obj, ilG_rendid id);
typedef void (*ilG_multiupdate_fn)(void *obj, ilG_rendid id, struct il_mat *mats);
typedef void (*ilG_draw_fn)(void *obj, ilG_rendid id, struct il_mat **mats, const unsigned *objects, unsigned num_mats);
typedef struct ilG_buildresult {
    ilG_free_fn free;
    ilG_update_fn update;
    ilG_draw_fn draw;
    ilG_multiupdate_fn view;
    int *types;
    unsigned num_types;
    void *obj;
} ilG_buildresult;

typedef bool (*ilG_build_fn)(void *obj, ilG_rendid id, struct ilG_context *context, ilG_buildresult *out);
typedef struct ilG_builder {
    ilG_build_fn build;
    void *obj;
} ilG_builder;

typedef struct ilG_handle {
    ilG_rendid id;
    struct ilG_context *context;
} ilG_handle;

typedef struct ilG_coordsys {
    ilG_free_fn free;
    void (*viewmats)(void *, struct il_mat *out, int *types, unsigned num_types);
    void (*objmats)(void *, const unsigned *objects, unsigned num_objects, struct il_mat *out, int type);
    void *obj;
    unsigned id;
} ilG_coordsys;

typedef bool (*ilG_coordsys_build_fn)(void *obj, unsigned id, struct ilG_context *context, ilG_coordsys *out);
typedef struct ilG_coordsys_builder {
    ilG_coordsys_build_fn build;
    void *obj;
} ilG_coordsys_builder;

typedef struct ilG_light {
    unsigned id;
    il_vec3 color;
    float radius;
} ilG_light;

typedef struct ilG_objrenderer {
    ilG_draw_fn draw;
    unsigned coordsys;
    int *types;
    unsigned num_types;
    IL_ARRAY(unsigned,) objects;
} ilG_objrenderer;

typedef struct ilG_viewrenderer {
    ilG_multiupdate_fn update;
    unsigned coordsys;
    int *types;
    unsigned num_types;
} ilG_viewrenderer;

typedef struct ilG_statrenderer {
    ilG_update_fn update;
} ilG_statrenderer;

struct ilG_renderer {
    ilG_free_fn free;
    IL_ARRAY(unsigned,) children;
    IL_ARRAY(ilG_light,) lights;
    unsigned obj, view, stat;
    void *data;
};

il_pair(ilG_rendstorage,ilG_rendid, il_table);
il_pair(ilG_rendname,   ilG_rendid, unsigned);

typedef struct ilG_rendermanager {
    IL_ARRAY(ilG_renderer,)     renderers;
    IL_ARRAY(ilG_objrenderer,)  objrenderers;
    IL_ARRAY(ilG_viewrenderer,) viewrenderers;
    IL_ARRAY(ilG_statrenderer,) statrenderers;
    IL_ARRAY(ilG_rendid,)       rendids;
    IL_ARRAY(ilG_msgsink,)      sinks;
    IL_ARRAY(ilG_rendstorage,)  storages;
    IL_ARRAY(ilG_rendname,)     namelinks;
    IL_ARRAY(char*,)            names;
    IL_ARRAY(ilG_coordsys,)     coordsystems;
    ilG_rendid curid;
    ilG_cosysid cursysid;
} ilG_rendermanager;

typedef struct ilG_legacy ilG_legacy;
#define ilG_legacy_builder(p) ilG_builder_wrap(p, ilG_legacy_build)
ilG_legacy *ilG_legacy_new(struct ilG_drawable3d *dr, struct ilG_material *mtl);
void ilG_legacy_addTexture(ilG_legacy *self, struct ilG_tex tex);
void ilG_legacy_addMatrix(ilG_legacy *self, unsigned loc, int type);
bool ilG_legacy_build(void *ptr, ilG_rendid id, struct ilG_context *context, ilG_buildresult *out);

ilG_builder ilG_builder_wrap(void *obj, const ilG_build_fn build);

ilG_handle ilG_build(ilG_builder self, struct ilG_context *context);
ilG_cosysid ilG_coordsys_build(ilG_coordsys_builder self, struct ilG_context *context);
void ilG_handle_destroy(ilG_handle self);
bool ilG_handle_ready(ilG_handle self);
il_table *ilG_handle_storage(ilG_handle self);
const char *ilG_handle_getName(ilG_handle self);
void ilG_handle_addCoords(ilG_handle self, ilG_cosysid cosys, unsigned codata);
void ilG_handle_delCoords(ilG_handle self, ilG_cosysid cosys, unsigned codata);
void ilG_handle_setViewCoords(ilG_handle self, ilG_cosysid cosys);
void ilG_handle_addRenderer(ilG_handle self, ilG_handle node);
void ilG_handle_delRenderer(ilG_handle self, ilG_handle node);
void ilG_handle_addLight(ilG_handle self, ilG_light light);
void ilG_handle_delLight(ilG_handle self, ilG_light light);
void ilG_handle_message(ilG_handle self, int type, il_value v);

#endif
