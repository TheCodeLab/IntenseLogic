#include "context.h"
#include "context-internal.h"

#include <limits.h>

#include "util/log.h"

/////////////////////////////////////////////////////////////////////////////
// Message wrappers

bool ilG_context_upload(ilG_context *self, void (*fn)(void*), void* ptr)
{
    ilG_context_msg msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = ILG_UPLOAD;
    msg.v.upload.cb = fn;
    msg.v.upload.ptr = ptr;
    ilG_context_queue_produce(self->queue, msg);
    return true;
}

bool ilG_context_resize(ilG_context *self, int w, int h)
{
    ilG_context_msg msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = ILG_RESIZE;
    msg.v.resize[0] = w;
    msg.v.resize[1] = h;
    ilG_context_queue_produce(self->queue, msg);
    return true;
}

char *strdup(const char*);
bool ilG_context_rename(ilG_context *self, const char *title)
{
    SDL_SetWindowTitle(self->window, title);
    return true;
}

void ilG_context_setNotifier(ilG_context *self, void (*fn)(il_value*), il_value val)
{
    self->client->notify = fn;
    self->client->user = val;
}

/////////////////////////////////////////////////////////////////////////////
// Access and modification

void ilG_context_message(ilG_context *self, ilG_rendid id, int type, il_value val)
{
    ilG_client_msg msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = ILG_CLIENT_MESSAGE;
    msg.v.message.id = id;
    msg.v.message.type = type;
    msg.v.message.data = val;
    ilG_client_queue_produce(self->client, msg);
}

ilG_renderer *ilG_context_findRenderer(ilG_context *self, ilG_rendid id)
{
    ilG_rendid key;
    unsigned idx;
    IL_FIND(self->manager.rendids, key, key == id, idx);
    if (idx < self->manager.rendids.length) {
        return &self->manager.renderers.data[idx];
    }
    return NULL;
}

ilG_msgsink *ilG_context_findSink(ilG_context *self, ilG_rendid id)
{
    ilG_msgsink r;
    unsigned idx;
    IL_FIND(self->manager.sinks, r, r.id == id, idx);
    if (idx < self->manager.sinks.length) {
        return &self->manager.sinks.data[idx];
    }
    return NULL;
}

il_table *ilG_context_findStorage(ilG_context *self, ilG_rendid id)
{
    ilG_rendstorage r;
    unsigned idx;
    IL_FIND(self->manager.storages, r, r.first == id, idx);
    if (idx < self->manager.storages.length) {
        return &self->manager.storages.data[idx].second;
    }
    return NULL;
}

const char *ilG_context_findName(ilG_context *self, ilG_rendid id)
{
    ilG_rendname r;
    unsigned idx;
    IL_FIND(self->manager.namelinks, r, r.first == id, idx);
    if (idx < self->manager.namelinks.length) {
        return self->manager.names.data[r.second];
    }
    return NULL;
}

const char *ilG_context_findError(ilG_context *self, ilG_rendid id)
{
    ilG_error e;
    unsigned idx;
    IL_FIND(self->manager.failed, e, e.first == id, idx);
    if (idx < self->manager.failed.length) {
        return e.second;
    }
    return NULL;
}

ilG_material *ilG_context_findMaterial(ilG_context *self, ilG_matid mat)
{
    return &self->manager.materials.data[mat.id];
}

unsigned ilG_context_addRenderer(ilG_context *self, ilG_rendid id, ilG_builder builder)
{
    ilG_buildresult b;
    memset(&b, 0, sizeof(ilG_buildresult));
    bool res = builder.build(builder.obj, id, self, &b);
    ilG_renderer r = (ilG_renderer) {
        .free = NULL,
        .children = {0,0,0},
        .lights = {0,0,0},
        .obj = 0,
        .view = 0,
        .stat = 0,
        .data = NULL
    };

    if (!res) {
        ilG_error e = (ilG_error) {id, b.error};
        il_error("Renderer failed: %s", b.error);
        IL_APPEND(self->manager.failed, e);
        ilG_client_msg msg;
        memset(&msg, 0, sizeof(msg));
        msg.type = ILG_FAILURE;
        msg.v.failure.id = id;
        msg.v.failure.msg = b.error;
        ilG_client_queue_produce(self->client, msg);
        return UINT_MAX;
    }

    r.free = b.free;
    r.data = b.obj;
    if (b.update) {
        ilG_statrenderer s = (ilG_statrenderer) {
            .update = b.update
        };
        r.stat = self->manager.statrenderers.length;
        IL_APPEND(self->manager.statrenderers, s);
    }
    if (b.view) {
        for (unsigned i = 0; i < b.num_types; i++) {
            if (b.types[i] & ILG_MODEL) {
                il_error("View renderers cannot have model transformations");
                return UINT_MAX;
            }
        }
        ilG_viewrenderer v = (ilG_viewrenderer) {
            .update = b.view,
            .coordsys = 0, // TODO: Select coord system
            .types = b.types,
            .num_types = b.num_types,
        };
        r.view = self->manager.viewrenderers.length;
        IL_APPEND(self->manager.viewrenderers, v);
    }
    if (b.draw) {
        ilG_objrenderer m = (ilG_objrenderer) {
            .draw = b.draw,
            .coordsys = 0, // TODO: Select coord system
            .types = b.types,
            .num_types = b.num_types
        };
        r.obj = self->manager.objrenderers.length;
        IL_APPEND(self->manager.objrenderers, m);
    }
    IL_APPEND(self->manager.renderers, r);
    IL_APPEND(self->manager.rendids, id);
    ilG_client_msg msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = ILG_READY;
    msg.v.ready = id;
    ilG_client_queue_produce(self->client, msg);
    return self->manager.renderers.length - 1;
}

unsigned ilG_context_addSink(ilG_context *self, ilG_rendid id, ilG_message_fn sink)
{
    ilG_msgsink s = {
        .fn = sink,
        .id = id
    };
    IL_APPEND(self->manager.sinks, s);
    return self->manager.sinks.length - 1;
}

bool ilG_context_addChild(ilG_context *self, ilG_rendid parent, ilG_rendid child)
{
    ilG_renderer *r = ilG_context_findRenderer(self, parent);
    if (!r) {
        return false;
    }
    unsigned idx;
    ilG_rendid id;
    IL_FIND(self->manager.rendids, id, id == child, idx);
    if (idx < self->manager.rendids.length) {
        IL_APPEND(r->children, idx);
        return true;
    }
    return false;
}

unsigned ilG_context_addCoords(ilG_context *self, ilG_rendid id, ilG_cosysid cosysid, unsigned codata)
{
    ilG_renderer *r = ilG_context_findRenderer(self, id);
    if (!r) {
        return UINT_MAX;
    }
    ilG_objrenderer *or = &self->manager.objrenderers.data[r->obj];
    unsigned cosys;
    ilG_coordsys co;
    IL_FIND(self->manager.coordsystems, co, co.id == cosysid, cosys);
    if (cosys == self->manager.coordsystems.length) {
        il_error("No such coord system");
        return UINT_MAX;
    }
    if (or->coordsys != cosys) {
        or->coordsys = cosys;
        if (or->objects.length > 0) {
            il_warning("Multiple coord systems per renderer not supported: Overwriting");
        }
        or->objects.length = 0;
    }
    IL_APPEND(or->objects, codata);
    return or->objects.length - 1;
}

bool ilG_context_viewCoords(ilG_context *self, ilG_rendid id, ilG_cosysid cosys)
{
    ilG_renderer *r = ilG_context_findRenderer(self, id);
    if (!r) {
        return false;
    }
    ilG_viewrenderer *vr = &self->manager.viewrenderers.data[r->view];
    vr->coordsys = cosys;
    return true;
}

unsigned ilG_context_addLight(ilG_context *self, ilG_rendid id, ilG_light light)
{
    ilG_renderer *r = ilG_context_findRenderer(self, id);
    if (!r) {
        return UINT_MAX;
    }
    IL_APPEND(r->lights, light);
    return r->lights.length-1;
}

unsigned ilG_context_addStorage(ilG_context *self, ilG_rendid id)
{
    ilG_rendstorage r = {
        .first = id,
        .second = il_table_new()
    };
    IL_APPEND(self->manager.storages, r);
    return self->manager.storages.length - 1;
}

unsigned ilG_context_addName(ilG_context *self, ilG_rendid id, const char *name)
{
    unsigned idx;
    IL_FIND(self->manager.names, const char *s, strcmp(s, name) == 0, idx);
    if (idx == self->manager.names.length) {
        char *s = strdup(name);
        IL_APPEND(self->manager.names, s);
    }
    ilG_rendname r = {
        .first = id,
        .second = idx
    };
    IL_APPEND(self->manager.namelinks, r);
    return self->manager.namelinks.length - 1;
}

unsigned ilG_context_addCoordSys(ilG_context *self, ilG_coordsys co)
{
    IL_APPEND(self->manager.coordsystems, co);
    return self->manager.coordsystems.length-1;
}

ilG_matid ilG_context_addMaterial(ilG_context *self, ilG_material mat)
{
    IL_APPEND(self->manager.materials, mat);
    return (ilG_matid){self->manager.materials.length-1};
}

bool ilG_context_delRenderer(ilG_context *self, ilG_rendid id)
{
    unsigned idx;
    ilG_rendid key;
    IL_FIND(self->manager.rendids, key, key == id, idx);
    if (idx < self->manager.rendids.length) {
        ilG_renderer *r = &self->manager.renderers.data[idx];
        r->free(r->data);
        IL_FREE(r->children);
        IL_FREE(r->lights);
        if (r->obj) {
            ilG_objrenderer *or = &self->manager.objrenderers.data[r->obj];
            free(or->types);
            IL_FREE(or->objects);
        }
        if (r->view) {
            ilG_viewrenderer *vr = &self->manager.viewrenderers.data[r->view];
            free(vr->types);
        }
        // TODO: Stop leaking {obj,view,stat}renderer memory in array, use freelist or something
        IL_FASTREMOVE(self->manager.renderers, idx);
        IL_FASTREMOVE(self->manager.rendids, idx);
        return true;
    }
    return false;
}

bool ilG_context_delSink(ilG_context *self, ilG_rendid id)
{
    unsigned idx;
    ilG_msgsink r;
    IL_FIND(self->manager.sinks, r, r.id == id, idx);
    if (idx < self->manager.sinks.length) {
        IL_FASTREMOVE(self->manager.sinks, idx);
        return true;
    }
    return false;
}

bool ilG_context_delChild(ilG_context *self, ilG_rendid parent, ilG_rendid child)
{
    unsigned idx;
    ilG_rendid id;
    ilG_renderer *par = ilG_context_findRenderer(self, parent);
    IL_FIND(self->manager.rendids, id, id == child, idx);
    if (idx < self->manager.rendids.length) {
        unsigned idx2;
        IL_FIND(par->children, id, id == idx, idx2);
        if (idx2 < par->children.length) {
            IL_FASTREMOVE(par->children, idx2);
            return true;
        }
    }
    return false;
}

bool ilG_context_delCoords(ilG_context *self, ilG_rendid id, unsigned cosys, unsigned codata)
{
    ilG_renderer *r = ilG_context_findRenderer(self, id);
    if (!r) {
        return false;
    }
    ilG_objrenderer *or = &self->manager.objrenderers.data[r->obj];
    unsigned idx;
    unsigned d;
    if (or->coordsys != cosys) {
        return false;
    }
    IL_FIND(or->objects, d, d == codata, idx);
    if (idx < or->objects.length) {
        IL_FASTREMOVE(or->objects, idx);
        return true;
    }
    return false;
}

bool ilG_context_delLight(ilG_context *self, ilG_rendid id, ilG_light light)
{
    ilG_renderer *r = ilG_context_findRenderer(self, id);
    if (!r) {
        return false;
    }
    unsigned idx;
    ilG_light val;
    IL_FIND(r->lights, val, val.id == light.id, idx);
    if (idx < r->lights.length) {
        IL_FASTREMOVE(r->lights, idx);
        return true;
    }
    return false;
}

bool ilG_context_delStorage(ilG_context *self, ilG_rendid id)
{
    unsigned idx;
    ilG_rendstorage r;
    IL_FIND(self->manager.storages, r, r.first == id, idx);
    if (idx < self->manager.storages.length) {
        IL_FASTREMOVE(self->manager.storages, idx);
        return true;
    }
    return false;
}

bool ilG_context_delName(ilG_context *self, ilG_rendid id)
{
    unsigned idx;
    ilG_rendname r;
    IL_FIND(self->manager.namelinks, r, r.first == id, idx);
    if (idx < self->manager.namelinks.length) {
        IL_REMOVE(self->manager.namelinks, idx);
        return true;
    }
    return false;
}

bool ilG_context_delCoordSys(ilG_context *self, unsigned id)
{
    unsigned idx;
    ilG_coordsys co;
    IL_FIND(self->manager.coordsystems, co, co.id == id, idx);
    if (idx < self->manager.coordsystems.length) {
        IL_FASTREMOVE(self->manager.coordsystems, idx);
        return true;
    }
    return false;
}

bool ilG_context_delMaterial(ilG_context *self, ilG_matid mat)
{
    (void)self, (void)mat;
    return false; // TODO: Material deletion
}
