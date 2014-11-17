#ifndef ILG_RENDERER_H
#define ILG_RENDERER_H

#include "util/storage.h"
#include "util/event.h"
#include "math/matrix.h"
#include "graphics/material.h"

struct ilG_context;
struct ilG_renderer;

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
    char *error;
    char *name;
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

typedef struct ilG_matid {
    unsigned id;
} ilG_matid;

il_pair(ilG_rendstorage,ilG_rendid, il_table);
il_pair(ilG_rendname,   ilG_rendid, unsigned);
il_pair(ilG_error,      ilG_rendid, char*);

typedef struct ilG_renderman {
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
    IL_ARRAY(ilG_error,)        failed;
    IL_ARRAY(ilG_material,)     materials;
    ilG_rendid curid;
    ilG_cosysid cursysid;
    ilE_handler material_creation;
} ilG_renderman;

void ilG_renderman_free(ilG_renderman *rm);
ilG_builder ilG_builder_wrap(void *obj, const ilG_build_fn build);
ilG_handle ilG_build(ilG_builder self, struct ilG_context *context);
ilG_cosysid ilG_coordsys_build(ilG_coordsys_builder self, struct ilG_context *context);
void ilG_handle_destroy(ilG_handle self);
bool ilG_handle_ready(ilG_handle self);
il_table *ilG_handle_storage(ilG_handle self);
const char *ilG_handle_getName(ilG_handle self);
const char *ilG_handle_getError(ilG_handle self);
void ilG_handle_addCoords(ilG_handle self, ilG_cosysid cosys, unsigned codata);
void ilG_handle_delCoords(ilG_handle self, ilG_cosysid cosys, unsigned codata);
void ilG_handle_setViewCoords(ilG_handle self, ilG_cosysid cosys);
void ilG_handle_addRenderer(ilG_handle self, ilG_handle node);
void ilG_handle_delRenderer(ilG_handle self, ilG_handle node);
void ilG_handle_addLight(ilG_handle self, ilG_light light);
void ilG_handle_delLight(ilG_handle self, ilG_light light);
void ilG_handle_message(ilG_handle self, int type, il_value v);

/* Rendering thread calls */
void ilG_renderman_message(ilG_renderman *self, ilG_rendid id, int type, il_value val);
ilG_renderer    *ilG_renderman_findRenderer       (ilG_renderman *self, ilG_rendid id);
ilG_msgsink     *ilG_renderman_findSink           (ilG_renderman *self, ilG_rendid id);
il_table        *ilG_renderman_findStorage        (ilG_renderman *self, ilG_rendid id);
const char      *ilG_renderman_findName           (ilG_renderman *self, ilG_rendid id);
const char      *ilG_renderman_findError          (ilG_renderman *self, ilG_rendid id);
ilG_material    *ilG_renderman_findMaterial       (ilG_renderman *self, ilG_matid mat);
unsigned  ilG_renderman_addSink     (ilG_renderman *self, ilG_rendid id, ilG_message_fn sink);
bool      ilG_renderman_addChild    (ilG_renderman *self, ilG_rendid parent, ilG_rendid child);
unsigned  ilG_renderman_addCoords   (ilG_renderman *self, ilG_rendid id, ilG_cosysid cosys, unsigned codata);
bool      ilG_renderman_viewCoords  (ilG_renderman *self, ilG_rendid id, ilG_cosysid cosys);
unsigned  ilG_renderman_addLight    (ilG_renderman *self, ilG_rendid id, struct ilG_light light);
unsigned  ilG_renderman_addStorage  (ilG_renderman *self, ilG_rendid id);
unsigned  ilG_renderman_addName     (ilG_renderman *self, ilG_rendid id, const char *name);
unsigned  ilG_renderman_addCoordSys (ilG_renderman *self, ilG_coordsys co);
ilG_matid ilG_renderman_addMaterial (ilG_renderman *self, ilG_material mat);
bool ilG_renderman_delRenderer      (ilG_renderman *self, ilG_rendid id);
bool ilG_renderman_delSink          (ilG_renderman *self, ilG_rendid id);
bool ilG_renderman_delChild         (ilG_renderman *self, ilG_rendid parent, ilG_rendid child);
bool ilG_renderman_delCoords        (ilG_renderman *self, ilG_rendid id, ilG_cosysid cosys, unsigned codata);
bool ilG_renderman_delLight         (ilG_renderman *self, ilG_rendid id, struct ilG_light light);
bool ilG_renderman_delStorage       (ilG_renderman *self, ilG_rendid id);
bool ilG_renderman_delName          (ilG_renderman *self, ilG_rendid id);
bool ilG_renderman_delCoordSys      (ilG_renderman *self, unsigned id);
bool ilG_renderman_delMaterial      (ilG_renderman *self, ilG_matid mat);

void ilG_material_print(ilG_material *mat);
void ilG_renderer_print(struct ilG_context *c, ilG_rendid root, unsigned depth);
void ilG_renderman_print(struct ilG_context *c, ilG_rendid root);

#endif
