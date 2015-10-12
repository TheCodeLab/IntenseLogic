#ifndef ILG_RENDERER_H
#define ILG_RENDERER_H

#include "util/storage.h"
#include "util/event.h"
#include "math/matrix.h"
#include "graphics/material.h"
#include "graphics/tex.h"
#include "graphics/mesh.h"

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

typedef struct ilG_shadowdata {
    GLuint vao;
    unsigned first, count;
    GLenum type;
} ilG_shadowdata;

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
    IL_ARRAY(ilG_shader,)       shaders;
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
ilG_shader      *ilG_renderman_findShader         (ilG_renderman *self, unsigned id);
unsigned  ilG_renderman_addSink     (ilG_renderman *self, ilG_rendid id, ilG_message_fn sink);
bool      ilG_renderman_addChild    (ilG_renderman *self, ilG_rendid parent, ilG_rendid child);
unsigned  ilG_renderman_addCoords   (ilG_renderman *self, ilG_rendid id, ilG_cosysid cosys, unsigned codata);
bool      ilG_renderman_viewCoords  (ilG_renderman *self, ilG_rendid id, ilG_cosysid cosys);
unsigned  ilG_renderman_addLight    (ilG_renderman *self, ilG_rendid id, struct ilG_light light);
unsigned  ilG_renderman_addStorage  (ilG_renderman *self, ilG_rendid id);
unsigned  ilG_renderman_addName     (ilG_renderman *self, ilG_rendid id, const char *name);
unsigned  ilG_renderman_addCoordSys (ilG_renderman *self, ilG_coordsys co);
ilG_matid ilG_renderman_addMaterial (ilG_renderman *self, ilG_material mat);
__attribute__((warn_unused_result))
bool ilG_renderman_addMaterialFromShader(ilG_renderman *self, ilG_material mat, ilG_shader vert,
                                         ilG_shader frag, ilG_matid *out, char **error);
__attribute__((warn_unused_result))
bool ilG_renderman_addMaterialFromFile(ilG_renderman *self, ilG_material mat, const char *vert,
                                       const char *frag, ilG_matid *out, char **error);
unsigned  ilG_renderman_addShader   (ilG_renderman *self, ilG_shader shader);
bool ilG_renderman_delRenderer      (ilG_renderman *self, ilG_rendid id);
bool ilG_renderman_delSink          (ilG_renderman *self, ilG_rendid id);
bool ilG_renderman_delChild         (ilG_renderman *self, ilG_rendid parent, ilG_rendid child);
bool ilG_renderman_delCoords        (ilG_renderman *self, ilG_rendid id, ilG_cosysid cosys, unsigned codata);
bool ilG_renderman_delLight         (ilG_renderman *self, ilG_rendid id, struct ilG_light light);
bool ilG_renderman_delStorage       (ilG_renderman *self, ilG_rendid id);
bool ilG_renderman_delName          (ilG_renderman *self, ilG_rendid id);
bool ilG_renderman_delCoordSys      (ilG_renderman *self, unsigned id);
bool ilG_renderman_delMaterial      (ilG_renderman *self, ilG_matid mat);
bool ilG_renderman_delShader        (ilG_renderman *self, unsigned id);

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
struct ilA_mesh;
struct ilG_shape;

void ilG_geometry_bind(tgl_fbo *gbuffer);
ilG_builder ilG_geometry_builder(struct ilG_context *context);

typedef struct ilG_skybox {
    ilG_tex texture;
    ilG_renderman *rm;
    ilG_matid mat;
} ilG_skybox;

bool ilG_skybox_build(ilG_skybox *skybox, ilG_renderman *rm, ilG_tex skytex, char **error);
void ilG_skybox_draw(ilG_skybox *skybox, il_mat vp);
void ilG_skybox_free(ilG_skybox *skybox);
ilG_builder ilG_skybox_builder(ilG_skybox *skybox, ilG_tex skytex);

typedef enum ilG_light_type {
    ILG_POINT,
    ILG_SUN,
} ilG_light_type;

typedef struct ilG_lighting {
    // update these
    tgl_fbo *gbuffer, *accum;
    unsigned width, height;
    float fovsquared;
    // end
    ilG_renderman *rm;
    GLint lights_size, mvp_size, lights_offset[3], mvp_offset[1], color_loc, radius_loc, mvp_loc, mv_loc, ivp_loc, size_loc, fovsquared_loc;
    ilG_matid mat;
    struct ilG_shape *ico;
    tgl_vao vao;
    tgl_quad quad;
    // legacy
    ilG_light_type type;
    bool msaa;
} ilG_lighting;

bool ilG_lighting_build(ilG_lighting *lighting, ilG_renderman *rm,
                        ilG_light_type type, bool msaa, char **error);
void ilG_lighting_free(ilG_lighting *lighting);
/* ILG_INVERSE | ILG_VIEW_R | ILG_PROJECTION
   ILG_MODEL_T | ILG_VIEW_T
   ILG_MODEL_T | ILG_VP */
void ilG_lighting_draw(ilG_lighting *lighting, const il_mat *ivp, const il_mat *mv,
                       const il_mat *vp, const ilG_light *lights, size_t count);
ilG_builder ilG_lighting_builder(ilG_lighting *lighting, bool msaa, ilG_light_type type);

typedef struct ilG_tonemapper {
    // update these
    float exposure, gamma;
    // end
    unsigned w, h;
    struct ilG_context *context;
    ilG_renderman *rm;
    ilG_matid tonemap, horizblur, vertblur;
    tgl_fbo front, result;
    tgl_quad quad;
    tgl_vao vao;
    GLuint t_size_loc, h_size_loc, v_size_loc, t_exposure_loc, h_exposure_loc, gamma_loc;
} ilG_tonemapper;

bool ilG_tonemapper_build(ilG_tonemapper *tm, struct ilG_context *context, char **error);
void ilG_tonemapper_free(ilG_tonemapper *tm);
void ilG_tonemapper_resize(ilG_tonemapper *tm, unsigned w, unsigned h);
void ilG_tonemapper_draw(ilG_tonemapper *tm);
ilG_builder ilG_tonemapper_builder(ilG_tonemapper *tm, struct ilG_context *context);

typedef struct ilG_heightmap {
    ilG_tex height, normal, color;
    ilG_matid mat;
    ilG_mesh mesh;
    unsigned w,h;
    ilG_renderman *rm;
    GLenum mvp, imt, size;
} ilG_heightmap;

bool ilG_heightmap_build(ilG_heightmap *hm, ilG_renderman *rm, unsigned w, unsigned h,
                         ilG_tex height, ilG_tex normal, ilG_tex color, char **error);
void ilG_heightmap_free(ilG_heightmap *hm);
void ilG_heightmap_draw(ilG_heightmap *hm, il_mat mvp, il_mat imt);
ilG_builder ilG_heightmap_builder(ilG_heightmap *hm, unsigned w, unsigned h,
                                  ilG_tex height, ilG_tex normal, ilG_tex color);

typedef struct ilG_wireframe {
    unsigned count;
    ilG_matid mat;
    ilG_renderman *rm;
    tgl_vao vao;
    GLuint vbo;
    GLint mvp_loc, col_loc;
} ilG_wireframe;

bool ilG_wireframe_build(ilG_wireframe *wf, ilG_renderman *rm, char **error);
void ilG_wireframe_free(ilG_wireframe *wf);
// number of vertices (3*count floats)
void ilG_wireframe_data(ilG_wireframe *wf, const float *verts, size_t count);
void ilG_wireframe_draw(ilG_wireframe *wf, il_mat mvp, const float col[3]);

ilA_imgerr ilG_screenshot(struct ilG_context *context, ilA_img *out);

typedef struct ilG_ambient {
    // public
    il_vec3 color;
    // private
    ilG_matid mat;
    GLuint col_loc, fovsquared_loc;
    ilG_renderman *rm;
    struct ilG_context *context;
    tgl_quad quad;
    tgl_vao vao;
} ilG_ambient;

bool ilG_ambient_build(ilG_ambient *ambient, struct ilG_context *context, char **error);
void ilG_ambient_free(ilG_ambient *ambient);
void ilG_ambient_draw(ilG_ambient *ambient);
ilG_builder ilG_ambient_builder(ilG_ambient *ambient, struct ilG_context *context);

#endif
