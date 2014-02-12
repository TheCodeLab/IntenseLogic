#include "guipass.h"

#include "graphics/gui.h"
#include "graphics/context.h"
#include "graphics/glutil.h"
#include "graphics/renderer.h"

struct ilG_gui {
    ilG_context *context;
    ilG_gui_frame *root;
    il_table storage;
};

static void gui_draw(void *ptr)
{
    ilG_gui *self = ptr;
    ilG_testError("Unknown");
    if (self->root) {
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        //glEnable(GL_BLEND);
        //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        self->root->rect = (ilG_gui_rect){
            .a = {0, 0, 0.f, 0.f},
            .b = {self->context->width, self->context->height, 0.f, 0.f}
        };
        ilG_gui_draw(self->root);
    }
    ilG_testError("gui_run");
}

static int gui_build(void *ptr, ilG_context *context)
{
    ilG_gui *self = ptr;
    self->context = context;
    return 1;
}

static il_table *gui_get_storage(void *ptr)
{
    ilG_gui *self = ptr;
    return &self->storage;
}

static bool gui_get_complete(const void *ptr)
{
    (void)ptr;
    return true;
}

const ilG_renderable ilG_gui_renderer = {
    .free = free,
    .draw = gui_draw,
    .build = gui_build,
    .get_storage = gui_get_storage,
    .get_complete = gui_get_complete,
    .add_positionable = NULL,
    .add_renderer = NULL,
    .name = "GUI"
};

ilG_gui *ilG_gui_new(ilG_gui_frame *root)
{
    ilG_gui *self = calloc(1, sizeof(ilG_gui));
    self->root = root;
    return self;
}

