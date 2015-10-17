#include "floatspace.h"

#include <assert.h>

#include "graphics/transform.h"
#include "util/array.h"

il_pos il_pos_new(ilG_floatspace *self)
{
    assert(self);
    if (self->free_list.length > 0) {
        return (il_pos) {
            .space = self,
            .id = self->free_list.data[--self->free_list.length]
        };
    }
    IL_APPEND(self->positions, il_vec3_new(0,0,0));
    IL_APPEND(self->rotations, il_quat_new(0,0,0,1));
    IL_APPEND(self->sizes, il_vec3_new(1,1,1));
    IL_APPEND(self->velocities, il_vec3_new(0,0,0));
    return (il_pos) {
        .space = self,
        .id = self->positions.length-1
    };
}

void il_pos_destroy(il_pos self)
{
    IL_APPEND(self.space->free_list, self.id);
}

il_vec3 il_pos_getPosition(const il_pos *self)
{
    assert(self->space);
    return self->space->positions.data[self->id];
}

il_quat il_pos_getRotation(const il_pos *self)
{
    assert(self->space);
    return self->space->rotations.data[self->id];
}

il_vec3 il_pos_getSize(const il_pos *self)
{
    assert(self->space);
    return self->space->sizes.data[self->id];
}

il_vec3 il_pos_getVelocity(const il_pos *self)
{
    assert(self->space);
    return self->space->velocities.data[self->id];
}

void il_pos_setPosition(il_pos *self, il_vec3 pos)
{
    assert(self->space);
    self->space->positions.data[self->id] = pos;
}

void il_pos_setRotation(il_pos *self, il_quat rot)
{
    assert(self->space);
    self->space->rotations.data[self->id] = rot;
}

void il_pos_setSize(il_pos *self, il_vec3 size)
{
    assert(self->space);
    self->space->sizes.data[self->id] = size;
}

void il_pos_setVelocity(il_pos *self, il_vec3 vel)
{
    assert(self->space);
    self->space->velocities.data[self->id] = vel;
}

ilG_floatspace *ilG_floatspace_new()
{
    ilG_floatspace *self = calloc(1, sizeof(ilG_floatspace));
    ilG_floatspace_init(self, 10);
    return self;
}

void ilG_floatspace_init(ilG_floatspace *self, size_t prealloc)
{
    memset(self, 0, sizeof(ilG_floatspace));
    IL_RESIZE(self->positions, prealloc);
    IL_RESIZE(self->rotations, prealloc);
    IL_RESIZE(self->sizes, prealloc);
    IL_RESIZE(self->velocities, prealloc);
    self->camera = il_pos_new(self);
    self->projection = il_mat_identity();
    pthread_mutex_init(&self->mtx, NULL);
}

void ilG_floatspace_free(ilG_floatspace *self)
{
    IL_FREE(self->positions);
    IL_FREE(self->rotations);
    IL_FREE(self->sizes);
    IL_FREE(self->velocities);
}

il_mat ilG_floatspace_viewmat(ilG_floatspace *self, int type)
{
    il_mat m = type & ILG_PROJECTION? self->projection : il_mat_identity();
    if (type & ILG_VIEW_R) {
        il_quat q = il_pos_getRotation(&self->camera);
        m = il_mat_mul(m, il_mat_rotate(q));
    }
    if (type & ILG_VIEW_T) {
        il_vec4 v = il_vec3_to_vec4(il_pos_getPosition(&self->camera), 1.0);
        v.x = -v.x;
        v.y = -v.y;
        v.z = -v.z;
        m = il_mat_mul(m, il_mat_translate(v));
    }
    if (type & ILG_INVERSE) {
        m = il_mat_invert(m);
    }
    if (type & ILG_TRANSPOSE) {
        m = il_mat_transpose(m);
    }
    return m;
}

void ilG_floatspace_objmats(ilG_floatspace *self, il_mat *out, const unsigned *objects, int type, size_t count)
{
#define mattype(matty) for (unsigned i = 0; i < count && (type & matty); i++)
    il_mat proj = self->projection;
    for (unsigned i = 0; i < count && !(type & ILG_PROJECTION); i++) {
        out[i] = il_mat_identity();
    }
    mattype(ILG_PROJECTION) {
        out[i] = proj;
    }
    il_mat viewr = il_mat_rotate(il_pos_getRotation(&self->camera));
    mattype(ILG_VIEW_R) {
        out[i] = il_mat_mul(out[i], viewr);
    }
    il_vec4 viewt_v = il_vec3_to_vec4(il_pos_getPosition(&self->camera), 1.0);
    viewt_v.x = -viewt_v.x;
    viewt_v.y = -viewt_v.y;
    viewt_v.z = -viewt_v.z;
    il_mat viewt = il_mat_translate(viewt_v);
    mattype(ILG_VIEW_T) {
        out[i] = il_mat_mul(out[i], viewt);
    }
    mattype(ILG_MODEL_T) {
        assert(objects[i] < self->positions.length);
        il_mat modelt = il_mat_translate(il_vec3_to_vec4(self->positions.data[objects[i]], 1.0));
        out[i] = il_mat_mul(out[i], modelt);
    }
    mattype(ILG_MODEL_R) {
        assert(objects[i] < self->rotations.length);
        il_mat modelr = il_mat_rotate(self->rotations.data[objects[i]]);
        out[i] = il_mat_mul(out[i], modelr);
    }
    mattype(ILG_MODEL_S) {
        assert(objects[i] < self->sizes.length);
        il_mat models = il_mat_scale(il_vec3_to_vec4(self->sizes.data[objects[i]], 1.0));
        out[i] = il_mat_mul(out[i], models);
    }
    mattype(ILG_INVERSE) {
        out[i] = il_mat_invert(out[i]);
    }
    mattype(ILG_TRANSPOSE) {
        out[i] = il_mat_transpose(out[i]);
    }
}

static void floatspace_viewmats(void *ptr, il_mat *out, int *types, unsigned count)
{
    for (unsigned i = 0; i < count; i++) {
        out[i] = ilG_floatspace_viewmat(ptr, types[i]);
    }
}

static void floatspace_objmats(void *ptr, const unsigned *objects, unsigned count, il_mat *out, int type)
{
    ilG_floatspace_objmats(ptr, out, objects, type, count);
}

void ilG_floatspace_build(ilG_floatspace *self, ilG_renderman *rm)
{
    self->rm = rm;
    ilG_coordsys sys;
    memset(&sys, 0, sizeof(sys));
    sys.viewmats = floatspace_viewmats;
    sys.objmats = floatspace_objmats;
    sys.obj = self;
    ilG_renderman_addCoordSys(self->rm, sys);
}

void ilG_floatspace_addPos(ilG_floatspace *self, ilG_rendid r, il_pos p)
{
    ilG_renderman_addCoords(self->rm, r, self->id, p.id);
}

void ilG_floatspace_delPos(ilG_floatspace *self, ilG_rendid r, il_pos p)
{
    ilG_renderman_delCoords(self->rm, r, self->id, p.id);
}
