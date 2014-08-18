#include "renderer.h"

#include <stdbool.h>

#include "graphics/context.h"
#include "common/storage.h"
#include "graphics/material.h"
#include "util/array.h"
#include "util/log.h"

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
