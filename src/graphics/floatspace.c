#include "floatspace.h"

#include "graphics/context.h"
#include "graphics/glutil.h"

ilG_floatspace *ilG_floatspace_new(il_world *world)
{
    ilG_floatspace *self = calloc(1, sizeof(ilG_floatspace));
    self->world = world? world : il_world_new(10);
    self->camera = il_positionable_new(self->world);
    self->projection = il_mat_identity();
    pthread_mutex_init(&self->mtx, NULL);
    return self;
}

static void floatspace_free(void *ptr)
{
    (void)ptr;
}

static void floatspace_viewmats(void *ptr, il_mat *out, int *types, unsigned num_types)
{
    ilG_floatspace *self = ptr;
    for (unsigned i = 0; i < num_types; i++) {
        unsigned f = types[i];
        il_mat m = f & ILG_PROJECTION? self->projection : il_mat_identity();
        if (f & ILG_VIEW_R) {
            il_quat q = il_positionable_getRotation(&self->camera);
            m = il_mat_mul(m, il_mat_rotate(q));
        }
        if (f & ILG_VIEW_T) {
            il_vec4 v = il_vec3_to_vec4(il_positionable_getPosition(&self->camera), 1.0);
            v.x = -v.x;
            v.y = -v.y;
            v.z = -v.z;
            m = il_mat_mul(m, il_mat_translate(v));
        }
        if (f & ILG_INVERSE) {
            m = il_mat_invert(m);
        }
        if (f & ILG_TRANSPOSE) {
            m = il_mat_transpose(m);
        }
        out[i] = m;
    }
}

static void floatspace_objmats(void *ptr, const unsigned *objects, unsigned num_objects, il_mat *out, int type)
{
    ilG_floatspace *self = ptr;
#define mattype(matty) for (unsigned i = 0; i < num_objects && (type & matty); i++)
    il_mat proj = self->projection;
    for (unsigned i = 0; i < num_objects && !(type & ILG_PROJECTION); i++) {
        out[i] = il_mat_identity();
    }
    mattype(ILG_PROJECTION) {
        out[i] = proj;
    }
    il_mat viewr = il_mat_rotate(il_positionable_getRotation(&self->camera));
    mattype(ILG_VIEW_R) {
        out[i] = il_mat_mul(out[i], viewr);
    }
    il_vec4 viewt_v = il_vec3_to_vec4(il_positionable_getPosition(&self->camera), 1.0);
    viewt_v.x = -viewt_v.x;
    viewt_v.y = -viewt_v.y;
    viewt_v.z = -viewt_v.z;
    il_mat viewt = il_mat_translate(viewt_v);
    mattype(ILG_VIEW_T) {
        out[i] = il_mat_mul(out[i], viewt);
    }
    mattype(ILG_MODEL_T) {
        il_positionable p = (il_positionable){ .id = objects[i], .world = self->world };
        il_mat modelt = il_mat_translate(il_vec3_to_vec4(il_positionable_getPosition(&p), 1.0));
        out[i] = il_mat_mul(out[i], modelt);
    }
    mattype(ILG_MODEL_R) {
        il_positionable p = (il_positionable){ .id = objects[i], .world = self->world };
        il_mat modelr = il_mat_rotate(il_positionable_getRotation(&p));
        out[i] = il_mat_mul(out[i], modelr);
    }
    mattype(ILG_MODEL_S) {
        il_positionable p = (il_positionable){ .id = objects[i], .world = self->world };
        il_mat models = il_mat_scale(il_vec3_to_vec4(il_positionable_getSize(&p), 1.0));
        out[i] = il_mat_mul(out[i], models);
    }
    mattype(ILG_INVERSE) {
        out[i] = il_mat_invert(out[i]);
    }
    mattype(ILG_TRANSPOSE) {
        out[i] = il_mat_transpose(out[i]);
    }
}

static bool floatspace_build(void *ptr, unsigned id, ilG_context *context, ilG_coordsys *out)
{
    (void)context;
    memset(out, 0, sizeof(ilG_coordsys));
    out->free = floatspace_free;
    out->viewmats = floatspace_viewmats;
    out->objmats = floatspace_objmats;
    out->obj = ptr;
    out->id = id;
    return true;
}

void ilG_floatspace_build(ilG_floatspace *self, ilG_context *context)
{
    self->id = ilG_coordsys_build((ilG_coordsys_builder){
        .build = floatspace_build,
        .obj = self
    }, context);
}

void ilG_floatspace_addPos(ilG_floatspace *self, ilG_handle r, il_positionable p)
{
    ilG_handle_addCoords(r, self->id, p.id);
}

void ilG_floatspace_delPos(ilG_floatspace *self, ilG_handle r, il_positionable p)
{
    ilG_handle_delCoords(r, self->id, p.id);
}
