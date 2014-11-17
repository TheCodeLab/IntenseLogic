#include "renderer.h"

#include <stdbool.h>
#include <limits.h>

#include "graphics/context.h"
#include "util/storage.h"
#include "graphics/material.h"
#include "util/array.h"
#include "util/log.h"

void ilG_renderman_free(ilG_renderman *rm)
{
#define foreach(list) for (unsigned i = 0; i < list.length; i++)
    foreach(rm->renderers) {
        rm->renderers.data[i].free(rm->renderers.data[i].data);
    }
    IL_FREE(rm->renderers);
    foreach(rm->objrenderers) {
        free(rm->objrenderers.data[i].types);
        IL_FREE(rm->objrenderers.data[i].objects);
    }
    IL_FREE(rm->objrenderers);
    foreach(rm->viewrenderers) {
        free(rm->viewrenderers.data[i].types);
    }
    IL_FREE(rm->viewrenderers);
    IL_FREE(rm->statrenderers);
    IL_FREE(rm->rendids);
    IL_FREE(rm->sinks);
    foreach(rm->storages) {
        il_table_free(rm->storages.data[i].second);
    }
    IL_FREE(rm->storages);
    IL_FREE(rm->namelinks);
    foreach(rm->names) {
        free(rm->names.data[i]);
    }
    IL_FREE(rm->names);
    foreach(rm->coordsystems) {
        rm->coordsystems.data[i].free(rm->coordsystems.data[i].obj);
    }
    IL_FREE(rm->coordsystems);
    foreach(rm->failed) {
        if (rm->failed.data[i].second) {
            free(rm->failed.data[i].second);
        }
    }
    IL_FREE(rm->failed);
}

ilG_builder ilG_builder_wrap(void *obj, const ilG_build_fn build)
{
    return (ilG_builder) {
        .obj = obj,
        .build = build
    };
}

struct build_ctx {
    ilG_builder self;
    ilG_context *context;
    unsigned id;
};
static void build(void *ptr)
{
    struct build_ctx *self = ptr;
    ilG_context_addRenderer(self->context, self->id, self->self);
    free(self);
}

ilG_handle ilG_build(ilG_builder self, ilG_context *context)
{
    struct build_ctx *ctx = calloc(1, sizeof(struct build_ctx));
    ctx->self = self;
    ctx->context = context;
    ctx->id = ++context->manager.curid;
    ilG_context_upload(context, build, ctx);
    return (ilG_handle) {
        .id = ctx->id,
        .context = context
    };
}

struct coordsys_ctx {
    ilG_coordsys_builder builder;
    ilG_context *context;
    ilG_cosysid id;
};
static void coordsys_build(void *ptr)
{
    struct coordsys_ctx *ctx = ptr;
    ilG_coordsys co;
    bool res = ctx->builder.build(ctx->builder.obj, ctx->context->manager.coordsystems.length, ctx->context, &co);
    co.id = ctx->id;
    if (res) {
        ilG_renderman_addCoordSys(&ctx->context->manager, co);
    }
    free(ctx);
}

ilG_cosysid ilG_coordsys_build(ilG_coordsys_builder self, struct ilG_context *context)
{
    struct coordsys_ctx *ctx = calloc(1, sizeof(struct coordsys_ctx));
    ctx->builder = self;
    ctx->context = context;
    ctx->id = ++context->manager.cursysid;
    ilG_context_upload(context, coordsys_build, ctx);
    return ctx->id;
}

bool ilG_handle_ready(ilG_handle self)
{
    unsigned idx;
    ilG_rendid id;
    IL_FIND(self.context->manager.rendids, id, id == self.id, idx);
    return idx < self.context->manager.renderers.length;
}

il_table *ilG_handle_storage(ilG_handle self)
{
    return ilG_renderman_findStorage(&self.context->manager, self.id);
}

const char *ilG_handle_getName(ilG_handle self)
{
    return ilG_renderman_findName(&self.context->manager, self.id);
}

#define log(...) do { fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); } while (0)
void ilG_material_print(ilG_material *mat)
{
    log("name: \"%s\"; vertex shader: %s; fragment shader: %s",
        mat->name, mat->vert.name, mat->frag.name);
}

void print_tabs(unsigned n)
{
    for (unsigned i = 0; i < n; i++) {
        fputs("  ", stderr);
    }
}

#define tabbedn(n, ...) do { print_tabs(n); log(__VA_ARGS__); } while (0)
#define tabbed(...) tabbedn(depth, __VA_ARGS__)
void ilG_renderer_print(ilG_context *c, ilG_rendid root, unsigned depth)
{
    ilG_renderman *rm = &c->manager;
    tabbed("Renderer %u: %s", root, ilG_renderman_findName(rm, root));
    ilG_renderer *r = ilG_renderman_findRenderer(rm, root);
    const char *error = ilG_renderman_findError(rm, root);
    if (error) {
        tabbed("FAILED: %s", error);
    }
    if (!r) {
        tabbed("Null renderer");
        return;
    }
    if (r->obj) {
        ilG_objrenderer *obj = &c->manager.objrenderers.data[r->obj];
        tabbed("coordsys: %u", obj->coordsys);
        print_tabs(depth);
        fputs("types: [ ", stderr);
        for (unsigned i = 0; i < obj->num_types; i++) {
            fprintf(stderr, "%u ", obj->types[i]);
        }
        fputs("]\n", stderr);
        print_tabs(depth);
        fputs("objects: [ ", stderr);
        for (unsigned i = 0; i < obj->objects.length; i++) {
            fprintf(stderr, "%u ", obj->objects.data[i]);
        }
        fputs("]\n", stderr);
    }
    if (r->view) {
        ilG_viewrenderer *view = &c->manager.viewrenderers.data[r->view];
        tabbed("coordsys: %u", view->coordsys);
        print_tabs(depth);
        fputs("types: [ ", stderr);
        for (unsigned i = 0; i < view->num_types; i++) {
            fprintf(stderr, "%u ", view->types[i]);
        }
        fputs("]\n", stderr);
    }
    if (r->lights.length > 0) {
        tabbed("lights: [");
        for (unsigned i = 0; i < r->lights.length; i++) {
            ilG_light *l = &r->lights.data[i];
            tabbedn(depth+1, "{ id: %u; color: (%.2f %.2f %.2f); radius: %f }",
                    l->id, l->color.x, l->color.y, l->color.z, l->radius);
        }
        tabbed("]");
    }
    if (r->children.length > 0) {
        tabbed("children: [");
        for (unsigned i = 0; i < r->children.length; i++) {
            ilG_renderer_print(c, c->manager.rendids.data[r->children.data[i]], depth + 1);
        }
        tabbed("]");
    } else {
        fputc('\n', stderr);
    }
}

void ilG_renderman_print(ilG_context *c, ilG_rendid root)
{
    ilG_renderman *rm = &c->manager;
    log("Materials:");
    for (unsigned i = 0; i < rm->materials.length; i++) {
        fprintf(stderr, "  %u: ", i);
        ilG_material_print(&rm->materials.data[i]);
    }
    ilG_renderer_print(c, root, 0);
}

/////////////////////////////////////////////////////////////////////////////
// Access and modification

ilG_renderer *ilG_renderman_findRenderer(ilG_renderman *self, ilG_rendid id)
{
    ilG_rendid key;
    unsigned idx;
    IL_FIND(self->rendids, key, key == id, idx);
    if (idx < self->rendids.length) {
        return &self->renderers.data[idx];
    }
    return NULL;
}

ilG_msgsink *ilG_renderman_findSink(ilG_renderman *self, ilG_rendid id)
{
    ilG_msgsink r;
    unsigned idx;
    IL_FIND(self->sinks, r, r.id == id, idx);
    if (idx < self->sinks.length) {
        return &self->sinks.data[idx];
    }
    return NULL;
}

il_table *ilG_renderman_findStorage(ilG_renderman *self, ilG_rendid id)
{
    ilG_rendstorage r;
    unsigned idx;
    IL_FIND(self->storages, r, r.first == id, idx);
    if (idx < self->storages.length) {
        return &self->storages.data[idx].second;
    }
    return NULL;
}

const char *ilG_renderman_findName(ilG_renderman *self, ilG_rendid id)
{
    ilG_rendname r;
    unsigned idx;
    IL_FIND(self->namelinks, r, r.first == id, idx);
    if (idx < self->namelinks.length) {
        return self->names.data[r.second];
    }
    return NULL;
}

const char *ilG_renderman_findError(ilG_renderman *self, ilG_rendid id)
{
    ilG_error e;
    unsigned idx;
    IL_FIND(self->failed, e, e.first == id, idx);
    if (idx < self->failed.length) {
        return e.second;
    }
    return NULL;
}

ilG_material *ilG_renderman_findMaterial(ilG_renderman *self, ilG_matid mat)
{
    return &self->materials.data[mat.id];
}

unsigned ilG_renderman_addSink(ilG_renderman *self, ilG_rendid id, ilG_message_fn sink)
{
    ilG_msgsink s = {
        .fn = sink,
        .id = id
    };
    IL_APPEND(self->sinks, s);
    return self->sinks.length - 1;
}

bool ilG_renderman_addChild(ilG_renderman *self, ilG_rendid parent, ilG_rendid child)
{
    ilG_renderer *r = ilG_renderman_findRenderer(self, parent);
    if (!r) {
        return false;
    }
    unsigned idx;
    ilG_rendid id;
    IL_FIND(self->rendids, id, id == child, idx);
    if (idx < self->rendids.length) {
        IL_APPEND(r->children, idx);
        return true;
    }
    return false;
}

unsigned ilG_renderman_addCoords(ilG_renderman *self, ilG_rendid id, ilG_cosysid cosysid, unsigned codata)
{
    ilG_renderer *r = ilG_renderman_findRenderer(self, id);
    if (!r) {
        return UINT_MAX;
    }
    ilG_objrenderer *or = &self->objrenderers.data[r->obj];
    unsigned cosys;
    ilG_coordsys co;
    IL_FIND(self->coordsystems, co, co.id == cosysid, cosys);
    if (cosys == self->coordsystems.length) {
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

bool ilG_renderman_viewCoords(ilG_renderman *self, ilG_rendid id, ilG_cosysid cosys)
{
    ilG_renderer *r = ilG_renderman_findRenderer(self, id);
    if (!r) {
        return false;
    }
    ilG_viewrenderer *vr = &self->viewrenderers.data[r->view];
    vr->coordsys = cosys;
    return true;
}

unsigned ilG_renderman_addLight(ilG_renderman *self, ilG_rendid id, ilG_light light)
{
    ilG_renderer *r = ilG_renderman_findRenderer(self, id);
    if (!r) {
        return UINT_MAX;
    }
    IL_APPEND(r->lights, light);
    return r->lights.length-1;
}

unsigned ilG_renderman_addStorage(ilG_renderman *self, ilG_rendid id)
{
    ilG_rendstorage r = {
        .first = id,
        .second = il_table_new()
    };
    IL_APPEND(self->storages, r);
    return self->storages.length - 1;
}

unsigned ilG_renderman_addName(ilG_renderman *self, ilG_rendid id, const char *name)
{
    unsigned idx;
    IL_FIND(self->names, const char *s, strcmp(s, name) == 0, idx);
    if (idx == self->names.length) {
        char *s = strdup(name);
        IL_APPEND(self->names, s);
    }
    ilG_rendname r = {
        .first = id,
        .second = idx
    };
    IL_APPEND(self->namelinks, r);
    return self->namelinks.length - 1;
}

unsigned ilG_renderman_addCoordSys(ilG_renderman *self, ilG_coordsys co)
{
    IL_APPEND(self->coordsystems, co);
    return self->coordsystems.length-1;
}

ilG_matid ilG_renderman_addMaterial(ilG_renderman *self, ilG_material mat)
{
    unsigned id = self->materials.length;
    IL_APPEND(self->materials, mat);
    il_value v = il_value_int(id);
    ilE_handler_fire(&self->material_creation, &v);
    return (ilG_matid){id};
}

bool ilG_renderman_delRenderer(ilG_renderman *self, ilG_rendid id)
{
    unsigned idx;
    ilG_rendid key;
    IL_FIND(self->rendids, key, key == id, idx);
    if (idx < self->rendids.length) {
        ilG_renderer *r = &self->renderers.data[idx];
        r->free(r->data);
        IL_FREE(r->children);
        IL_FREE(r->lights);
        if (r->obj) {
            ilG_objrenderer *or = &self->objrenderers.data[r->obj];
            free(or->types);
            IL_FREE(or->objects);
        }
        if (r->view) {
            ilG_viewrenderer *vr = &self->viewrenderers.data[r->view];
            free(vr->types);
        }
        // TODO: Stop leaking {obj,view,stat}renderer memory in array, use freelist or something
        IL_FASTREMOVE(self->renderers, idx);
        IL_FASTREMOVE(self->rendids, idx);
        return true;
    }
    return false;
}

bool ilG_renderman_delSink(ilG_renderman *self, ilG_rendid id)
{
    unsigned idx;
    ilG_msgsink r;
    IL_FIND(self->sinks, r, r.id == id, idx);
    if (idx < self->sinks.length) {
        IL_FASTREMOVE(self->sinks, idx);
        return true;
    }
    return false;
}

bool ilG_renderman_delChild(ilG_renderman *self, ilG_rendid parent, ilG_rendid child)
{
    unsigned idx;
    ilG_rendid id;
    ilG_renderer *par = ilG_renderman_findRenderer(self, parent);
    IL_FIND(self->rendids, id, id == child, idx);
    if (idx < self->rendids.length) {
        unsigned idx2;
        IL_FIND(par->children, id, id == idx, idx2);
        if (idx2 < par->children.length) {
            IL_FASTREMOVE(par->children, idx2);
            return true;
        }
    }
    return false;
}

bool ilG_renderman_delCoords(ilG_renderman *self, ilG_rendid id, unsigned cosys, unsigned codata)
{
    ilG_renderer *r = ilG_renderman_findRenderer(self, id);
    if (!r) {
        return false;
    }
    ilG_objrenderer *or = &self->objrenderers.data[r->obj];
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

bool ilG_renderman_delLight(ilG_renderman *self, ilG_rendid id, ilG_light light)
{
    ilG_renderer *r = ilG_renderman_findRenderer(self, id);
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

bool ilG_renderman_delStorage(ilG_renderman *self, ilG_rendid id)
{
    unsigned idx;
    ilG_rendstorage r;
    IL_FIND(self->storages, r, r.first == id, idx);
    if (idx < self->storages.length) {
        IL_FASTREMOVE(self->storages, idx);
        return true;
    }
    return false;
}

bool ilG_renderman_delName(ilG_renderman *self, ilG_rendid id)
{
    unsigned idx;
    ilG_rendname r;
    IL_FIND(self->namelinks, r, r.first == id, idx);
    if (idx < self->namelinks.length) {
        IL_REMOVE(self->namelinks, idx);
        return true;
    }
    return false;
}

bool ilG_renderman_delCoordSys(ilG_renderman *self, unsigned id)
{
    unsigned idx;
    ilG_coordsys co;
    IL_FIND(self->coordsystems, co, co.id == id, idx);
    if (idx < self->coordsystems.length) {
        IL_FASTREMOVE(self->coordsystems, idx);
        return true;
    }
    return false;
}

bool ilG_renderman_delMaterial(ilG_renderman *self, ilG_matid mat)
{
    (void)self, (void)mat;
    return false; // TODO: Material deletion
}
