#include "renderer.h"

#include <stdbool.h>
#include <limits.h>

#include "graphics/context.h"
#include "graphics/transform.h"
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
    ilG_renderman *rm;
    unsigned id;
};
static void build(void *ptr)
{
    struct build_ctx *self = ptr;
    ilG_renderman_addRenderer(self->rm, self->id, self->self);
    free(self);
}

ilG_handle ilG_build(ilG_builder self, ilG_renderman *rm)
{
    struct build_ctx *ctx = calloc(1, sizeof(struct build_ctx));
    ctx->self = self;
    ctx->rm = rm;
    ctx->id = ++rm->curid;
    ilG_renderman_upload(rm, build, ctx);
    return (ilG_handle) {
        .id = ctx->id,
        .rm = rm
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
    ilG_renderman_upload(&context->manager, coordsys_build, ctx);
    return ctx->id;
}

bool ilG_handle_ready(ilG_handle self)
{
    unsigned idx;
    ilG_rendid id;
    IL_FIND(self.rm->rendids, id, id == self.id, idx);
    return idx < self.rm->renderers.length;
}

const char *ilG_handle_getName(ilG_handle self)
{
    return ilG_renderman_findName(self.rm, self.id);
}

#define log(...) do { fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); } while (0)
void ilG_material_print(ilG_material *mat)
{
    log("name: \"%s\"; vertex shader ID: %u; fragment shader ID: %u",
        mat->name, mat->vert, mat->frag);
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
            fprintf(stderr, "%i ", obj->types[i]);
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
            fprintf(stderr, "%i ", view->types[i]);
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

ilG_shader *ilG_renderman_findShader(ilG_renderman *self, unsigned id)
{
    return &self->shaders.data[id];
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
    ilG_matid matid = (ilG_matid){id};
    self->material_creation(matid, self->material_creation_data);
    return matid;
}

__attribute__((warn_unused_result))
bool ilG_renderman_addMaterialFromShader(ilG_renderman *self, ilG_material mat, ilG_shader vert,
                                         ilG_shader frag, ilG_matid *out, char **error)
{
    mat.vert = ilG_renderman_addShader(self, vert);
    mat.frag = ilG_renderman_addShader(self, frag);
    if (!ilG_material_link(&mat, &vert, &frag, error)) {
        return false;
    }
    *out = ilG_renderman_addMaterial(self, mat);
    return true;
}

__attribute__((warn_unused_result))
bool ilG_renderman_addMaterialFromFile(ilG_renderman *self, ilG_material mat, const char *vertpath,
                                       const char *fragpath, ilG_matid *out, char **error)
{
    ilG_shader vert, frag;
    if (!ilG_shader_file(&vert, vertpath, GL_VERTEX_SHADER, error) ||
        !ilG_shader_file(&frag, fragpath, GL_FRAGMENT_SHADER, error) ||
        !ilG_shader_compile(&vert, error) ||
        !ilG_shader_compile(&frag, error)) {
        return false;
    }
    return ilG_renderman_addMaterialFromShader(self, mat, vert, frag, out, error);
}

unsigned ilG_renderman_addShader(ilG_renderman *self, ilG_shader shader)
{
    IL_APPEND(self->shaders, shader);
    return self->shaders.length - 1;
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

bool ilG_renderman_upload(ilG_renderman *self, void (*fn)(void*), void* ptr)
{
    ilG_renderman_msg msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = ILG_UPLOAD;
    msg.v.upload.cb = fn;
    msg.v.upload.ptr = ptr;
    ilG_renderman_queue_produce(&self->queue, msg);
    return true;
}

bool ilG_renderman_resize(ilG_renderman *self, int w, int h)
{
    ilG_renderman_msg msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = ILG_RESIZE;
    msg.v.resize[0] = w;
    msg.v.resize[1] = h;
    ilG_renderman_queue_produce(&self->queue, msg);
    return true;
}

unsigned ilG_renderman_addRenderer(ilG_renderman *self, ilG_rendid id, ilG_builder builder)
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
        IL_APPEND(self->failed, e);
        ilG_client_msg msg;
        memset(&msg, 0, sizeof(msg));
        msg.type = ILG_FAILURE;
        msg.v.failure.id = id;
        msg.v.failure.msg = b.error;
        ilG_client_queue_produce(&self->client, msg);
        return UINT_MAX;
    }

    r.free = b.free;
    r.data = b.obj;
    if (b.update) {
        ilG_statrenderer s = (ilG_statrenderer) {
            .update = b.update
        };
        r.stat = self->statrenderers.length;
        IL_APPEND(self->statrenderers, s);
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
        r.view = self->viewrenderers.length;
        IL_APPEND(self->viewrenderers, v);
    }
    if (b.draw) {
        ilG_objrenderer m = (ilG_objrenderer) {
            .draw = b.draw,
            .coordsys = 0, // TODO: Select coord system
            .types = b.types,
            .num_types = b.num_types
        };
        r.obj = self->objrenderers.length;
        IL_APPEND(self->objrenderers, m);
    }
    if (b.name) {
        ilG_renderman_addName(self, id, b.name);
        free(b.name);
    }
    IL_APPEND(self->renderers, r);
    IL_APPEND(self->rendids, id);
    ilG_client_msg msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = ILG_READY;
    msg.v.ready = id;
    ilG_client_queue_produce(&self->client, msg);
    return self->renderers.length - 1;
}

/////////////////////////////////////////////////////////////////////////////
// Renderman Queue

void ilG_renderman_queue_init(ilG_renderman_queue *queue)
{
    memset(queue, 0, sizeof(ilG_renderman_queue));
    pthread_mutex_init(&queue->mutex, NULL);
}

void ilG_renderman_queue_free(ilG_renderman_queue *queue)
{
    IL_FREE(queue->read);
    IL_FREE(queue->write);
    pthread_mutex_destroy(&queue->mutex);
}

void ilG_renderman_queue_produce(ilG_renderman_queue *queue, ilG_renderman_msg msg)
{
    pthread_mutex_lock(&queue->mutex);
    IL_APPEND(queue->write, msg);
    pthread_mutex_unlock(&queue->mutex);
}

void ilG_renderman_queue_read(ilG_renderman_queue *queue)
{
    pthread_mutex_lock(&queue->mutex);
    ilG_renderman_msgarray tmp = queue->read;
    queue->read = queue->write;
    tmp.length = 0;
    queue->write = tmp;
    pthread_mutex_unlock(&queue->mutex);
}

/////////////////////////////////////////////////////////////////////////////
// Client Queue

void ilG_client_queue_init(ilG_client_queue *queue)
{
    memset(queue, 0, sizeof(ilG_client_queue));
    pthread_mutex_init(&queue->mutex, NULL);
}

void ilG_client_queue_free(ilG_client_queue *queue)
{
    IL_FREE(queue->read);
    IL_FREE(queue->write);
    pthread_mutex_destroy(&queue->mutex);
}

void ilG_client_queue_produce(ilG_client_queue *queue, ilG_client_msg msg)
{
    pthread_mutex_lock(&queue->mutex);
    IL_APPEND(queue->write, msg);
    pthread_mutex_unlock(&queue->mutex);
    if (queue->notify) {
        queue->notify(&queue->user);
    }
}

void ilG_client_queue_read(ilG_client_queue *queue)
{
    pthread_mutex_lock(&queue->mutex);
    ilG_client_msgarray tmp = queue->read;
    queue->read = queue->write;
    tmp.length = 0;
    queue->write = tmp;
    pthread_mutex_unlock(&queue->mutex);
}
