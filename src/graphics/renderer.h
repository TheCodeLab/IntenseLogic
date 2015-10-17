#ifndef ILG_RENDERER_H
#define ILG_RENDERER_H

#include "math/matrix.h"
#include "graphics/material.h"
#include "graphics/tex.h"
#include "graphics/mesh.h"

enum ilG_gbuffer_attachments {
    ILG_GBUFFER_ALBEDO,     // Ratio of light reflected in RGB
    ILG_GBUFFER_NORMAL,     // Surface normal in camera-local space (world space centered on camera position)
    ILG_GBUFFER_REFRACTION, // Refractive index
    ILG_GBUFFER_GLOSS,      // How shiny or glossy the surface should be
    ILG_GBUFFER_EMISSION,   // Emission from the material itself, measured in radiant intensity (W/sr)
    ILG_GBUFFER_DEPTH,
    ILG_GBUFFER_NUMATTACHMENTS
};

struct ilG_renderer;
struct ilG_renderman;

#define il_pair(name, fst, snd) typedef struct name {fst first; snd second;} name

typedef unsigned ilG_rendid;
typedef unsigned ilG_cosysid;
typedef struct ilG_renderer ilG_renderer;

typedef void (*ilG_update_fn)(void *obj, ilG_rendid id);
typedef void (*ilG_multiupdate_fn)(void *obj, ilG_rendid id, struct il_mat *mats);
typedef void (*ilG_draw_fn)(void *obj, ilG_rendid id, struct il_mat **mats, const unsigned *objects, unsigned num_mats);
typedef struct ilG_buildresult {
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

typedef struct ilG_coordsys {
    void (*viewmats)(void *, struct il_mat *out, int *types, unsigned num_types);
    void (*objmats)(void *, const unsigned *objects, unsigned num_objects, struct il_mat *out, int type);
    void *obj;
    unsigned id;
} ilG_coordsys;

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
    IL_ARRAY(unsigned,) children;
    IL_ARRAY(ilG_light,) lights;
    unsigned obj, view, stat;
    void *data;
};

typedef struct ilG_matid {
    unsigned id;
} ilG_matid;

il_pair(ilG_rendname,   ilG_rendid, unsigned);
il_pair(ilG_error,      ilG_rendid, char*);

struct ilG_shape;

typedef struct ilG_renderman {
    IL_ARRAY(ilG_renderer,)     renderers;
    IL_ARRAY(ilG_objrenderer,)  objrenderers;
    IL_ARRAY(ilG_viewrenderer,) viewrenderers;
    IL_ARRAY(ilG_statrenderer,) statrenderers;
    IL_ARRAY(ilG_rendid,)       rendids;
    IL_ARRAY(ilG_rendname,)     namelinks;
    IL_ARRAY(char*,)            names;
    IL_ARRAY(ilG_coordsys,)     coordsystems;
    IL_ARRAY(ilG_error,)        failed;
    IL_ARRAY(ilG_material,)     materials;
    IL_ARRAY(ilG_shader,)       shaders;
    ilG_rendid curid;
    ilG_cosysid cursysid;
    void (*material_creation)(ilG_matid, void*);
    void *material_creation_data;
    unsigned width, height;
    tgl_fbo gbuffer, accum;
} ilG_renderman;

ilG_builder ilG_builder_wrap(void *obj, const ilG_build_fn build);

void ilG_renderman_free(ilG_renderman *rm);
void ilG_renderman_init(ilG_renderman *rm);
void ilG_renderman_setup(ilG_renderman *rm, bool msaa, bool hdr);
bool ilG_renderman_resize(ilG_renderman *self, int w, int h);

/* Rendering thread calls */
unsigned ilG_renderman_addRenderer(ilG_renderman *self, ilG_rendid id, ilG_builder builder);
ilG_renderer    *ilG_renderman_findRenderer       (ilG_renderman *self, ilG_rendid id);
const char      *ilG_renderman_findName           (ilG_renderman *self, ilG_rendid id);
const char      *ilG_renderman_findError          (ilG_renderman *self, ilG_rendid id);
ilG_material    *ilG_renderman_findMaterial       (ilG_renderman *self, ilG_matid mat);
ilG_shader      *ilG_renderman_findShader         (ilG_renderman *self, unsigned id);
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

void ilG_material_print(ilG_material *mat);
void ilG_renderer_print(ilG_renderman *rm, ilG_rendid root, unsigned depth);
void ilG_renderman_print(ilG_renderman *rm, ilG_rendid root);

struct ilA_img;
struct ilA_mesh;

typedef struct ilG_shape {
    GLuint vbo;
    GLuint ibo;
    GLuint vao;
    GLenum mode;
    GLsizei count;
} ilG_shape;

void ilG_box(ilG_shape *shape);
void ilG_icosahedron(ilG_shape *shape);
void ilG_shape_free(ilG_shape *shape);
void ilG_shape_bind(ilG_shape *shape);
void ilG_shape_draw(ilG_shape *shape);

void ilG_geometry_bind(tgl_fbo *gbuffer);
ilG_builder ilG_geometry_builder(ilG_renderman *rm);

typedef struct ilG_skybox {
    ilG_tex texture;
    ilG_renderman *rm;
    ilG_matid mat;
    ilG_shape *box;
} ilG_skybox;

bool ilG_skybox_build(ilG_skybox *skybox, ilG_renderman *rm, ilG_tex skytex, ilG_shape *box, char **error);
void ilG_skybox_draw(ilG_skybox *skybox, il_mat vp);
void ilG_skybox_free(ilG_skybox *skybox);
ilG_builder ilG_skybox_builder(ilG_skybox *skybox, ilG_tex skytex, ilG_shape *box);

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

bool ilG_lighting_build(ilG_lighting *lighting, ilG_renderman *rm, ilG_shape *ico,
                        ilG_light_type type, bool msaa, char **error);
void ilG_lighting_free(ilG_lighting *lighting);
/* ILG_INVERSE | ILG_VIEW_R | ILG_PROJECTION
   ILG_MODEL_T | ILG_VIEW_T
   ILG_MODEL_T | ILG_VP */
void ilG_lighting_draw(ilG_lighting *lighting, const il_mat *ivp, const il_mat *mv,
                       const il_mat *vp, const ilG_light *lights, size_t count);
ilG_builder ilG_lighting_builder(ilG_lighting *lighting, bool msaa, ilG_light_type type, ilG_shape *ico);

typedef struct ilG_tonemapper {
    // update these
    float exposure, gamma;
    bool debug_render;
    // end
    unsigned w, h;
    bool msaa;
    ilG_renderman *rm;
    ilG_matid tonemap, horizblur, vertblur;
    tgl_fbo front, result;
    tgl_quad quad;
    tgl_vao vao;
    GLuint t_size_loc, h_size_loc, v_size_loc, t_exposure_loc, h_exposure_loc, gamma_loc;
} ilG_tonemapper;

bool ilG_tonemapper_build(ilG_tonemapper *tm, ilG_renderman *rm, bool msaa, char **error);
void ilG_tonemapper_free(ilG_tonemapper *tm);
void ilG_tonemapper_resize(ilG_tonemapper *tm, unsigned w, unsigned h);
void ilG_tonemapper_draw(ilG_tonemapper *tm);
ilG_builder ilG_tonemapper_builder(ilG_tonemapper *tm, bool msaa);

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

ilA_imgerr ilG_screenshot(ilG_renderman *rm, ilA_img *out);

typedef struct ilG_ambient {
    // public
    il_vec3 color;
    float fovsquared;
    // private
    ilG_matid mat;
    GLuint col_loc, fovsquared_loc;
    ilG_renderman *rm;
    tgl_quad quad;
    tgl_vao vao;
} ilG_ambient;

bool ilG_ambient_build(ilG_ambient *ambient, ilG_renderman *rm, char **error);
void ilG_ambient_free(ilG_ambient *ambient);
void ilG_ambient_draw(ilG_ambient *ambient);
ilG_builder ilG_ambient_builder(ilG_ambient *ambient);

#endif
