#include "renderer.h"

#include <stdbool.h>

#include "graphics/context.h"
#include "util/storage.h"
#include "graphics/material.h"
#include "util/array.h"
#include "util/log.h"

void ilG_rendermanager_free(ilG_rendermanager *rm)
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
        ilG_context_addCoordSys(ctx->context, co);
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
    return ilG_context_findStorage(self.context, self.id);
}

const char *ilG_handle_getName(ilG_handle self)
{
    return ilG_context_findName(self.context, self.id);
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
    tabbed("Renderer %u: %s", root, ilG_context_findName(c, root));
    ilG_renderer *r = ilG_context_findRenderer(c, root);
    const char *error = ilG_context_findError(c, root);
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

void ilG_rendermanager_print(ilG_context *c, ilG_rendid root)
{
    ilG_rendermanager *rm = &c->manager;
    log("Materials:");
    for (unsigned i = 0; i < rm->materials.length; i++) {
        fprintf(stderr, "  %u: ", i);
        ilG_material_print(&rm->materials.data[i]);
    }
    ilG_renderer_print(c, root, 0);
}
