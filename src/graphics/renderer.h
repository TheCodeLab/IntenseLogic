#ifndef ILG_RENDERER_H
#define ILG_RENDERER_H

#include "util/storage.h"
#include "util/event.h"
#include "math/matrix.h"
#include "graphics/material.h"

struct ilG_context;
struct ilG_renderer;
struct ilG_renderman;

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

typedef bool (*ilG_build_fn)(void *obj, ilG_rendid id, struct ilG_renderman *renderman, ilG_buildresult *out);
typedef struct ilG_builder {
    ilG_build_fn build;
    void *obj;
} ilG_builder;

typedef struct ilG_handle {
    ilG_rendid id;
    struct ilG_renderman *rm;
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

typedef struct ilG_renderman_msg {
    enum ilG_renderman_msgtype {
        ILG_UPLOAD,
        ILG_RESIZE,
        ILG_STOP,
        ILG_END,
        ILG_MESSAGE,
        ILG_ADD_COORDS,
        ILG_DEL_COORDS,
        ILG_VIEW_COORDS,
        ILG_ADD_RENDERER,
        ILG_DEL_RENDERER,
        ILG_ADD_LIGHT,
        ILG_DEL_LIGHT
    } type;
    union {
        struct {
            void (*cb)(void*);
            void *ptr;
        } upload;
        int resize[2];
        struct {
            ilG_rendid node;
            int type;
            il_value data;
        } message;
        struct {
            ilG_rendid parent, child;
        } renderer;
        struct {
            ilG_rendid parent;
            unsigned cosys, codata;
        } coords;
        struct {
            ilG_rendid parent;
            ilG_light child;
        } light;
    } v;
} ilG_renderman_msg;

typedef struct ilG_client_msg {
    enum ilG_client_msgtype {
        ILG_READY,
        ILG_FAILURE,
        ILG_CLIENT_MESSAGE,
    } type;
    union {
        ilG_rendid ready;
        struct {
            ilG_rendid id;
            const char *msg;
        } failure;
        struct {
            ilG_rendid id;
            int type;
            il_value data;
        } message;
    } v;
} ilG_client_msg;

typedef IL_ARRAY(ilG_renderman_msg, ilG_renderman_msgarray) ilG_renderman_msgarray;

typedef struct ilG_renderman_queue {
    ilG_renderman_msgarray read, write;
    pthread_mutex_t mutex;
} ilG_renderman_queue;

typedef IL_ARRAY(ilG_client_msg, ilG_client_msgarray) ilG_client_msgarray;

typedef struct ilG_client_queue {
    ilG_client_msgarray read, write;
    pthread_mutex_t mutex;
    void (*notify)(il_value *);
    il_value user;
} ilG_client_queue;

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
    ilG_renderman_queue queue;
    ilG_client_queue client;
    il_table storage;
} ilG_renderman;

void ilG_renderman_free(ilG_renderman *rm);
ilG_builder ilG_builder_wrap(void *obj, const ilG_build_fn build);
ilG_handle ilG_build(ilG_builder self, struct ilG_renderman *renderman);
/** Calls a function at the beginning of the frame on the context thread, usually for building VBOs */
bool ilG_renderman_upload(ilG_renderman *self, void (*fn)(void*), void*);
/** Resizes (and creates if first call) the context's framebuffers and calls the #ilG_context.resize event.
 * @return Success. */
bool ilG_renderman_resize(ilG_renderman *self, int w, int h);
/** Sets the function called to notify client thread there are messages available */
void ilG_renderman_setNotifier(ilG_renderman *self, void (*fn)(il_value*), il_value val);

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
unsigned ilG_renderman_addRenderer(ilG_renderman *self, ilG_rendid id, ilG_builder builder);
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

void ilG_renderman_queue_init(ilG_renderman_queue *queue);
void ilG_renderman_queue_free(ilG_renderman_queue *queue);
void ilG_renderman_queue_produce(ilG_renderman_queue *queue, ilG_renderman_msg msg);
void ilG_renderman_queue_read(ilG_renderman_queue *queue);

void ilG_client_queue_init(ilG_client_queue *queue);
void ilG_client_queue_free(ilG_client_queue *queue);
void ilG_client_queue_produce(ilG_client_queue *queue, ilG_client_msg msg);
void ilG_client_queue_read(ilG_client_queue *queue);

void ilG_material_print(ilG_material *mat);
void ilG_renderer_print(struct ilG_context *c, ilG_rendid root, unsigned depth);
void ilG_renderman_print(struct ilG_context *c, ilG_rendid root);

struct ilA_img;
struct ilG_tex;

ilG_builder ilG_geometry_builder();
ilG_builder ilG_transparency_builder();
ilG_builder ilG_skybox_builder(struct ilG_tex skytex);
ilG_builder ilG_pointlight_builder(struct ilG_context *context);
ilG_builder ilG_sunlight_builder(struct ilG_context *context);
ilG_builder ilG_out_builder(struct ilG_context *context);
ilG_builder ilG_heightmap_builder(unsigned w, unsigned h, struct ilG_tex height, struct ilG_tex normal, struct ilG_tex color);
// takes ownership of verts
// verts is 3*count floats, or number of vertices
ilG_builder ilG_line_builder(unsigned num, float *verts, const float col[3]);
/** cb will only be called once */
ilG_builder ilG_grabber_builder(struct ilG_context *context, void (*cb)(struct ilA_img res, void *user), void *user);

#endif
