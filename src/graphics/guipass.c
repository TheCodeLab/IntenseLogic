#include "guipass.h"

#include "graphics/gui.h"
#include "graphics/stage.h"
#include "graphics/context.h"
#include "graphics/glutil.h"

struct ilG_gui {
    ilG_context *context;
    ilG_gui_frame *root;
};

static void gui_run(void *ptr)
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

static int gui_track(void *ptr, struct ilG_renderer *r)
{
    (void)ptr, (void)r;
    return 0;
}

const ilG_stagable ilG_gui_stage = {
    .run = gui_run,
    .track = gui_track,
    .name = "GUI"
};

ilG_gui *ilG_gui_new(struct ilG_context *context, ilG_gui_frame *root)
{
    ilG_gui *self = calloc(1, sizeof(ilG_gui));
    self->context = context;
    self->root = root;
    return self;
}

