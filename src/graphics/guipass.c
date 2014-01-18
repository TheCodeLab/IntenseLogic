#include "guipass.h"

#include "graphics/gui.h"
#include "graphics/stage.h"
#include "graphics/context.h"
#include "graphics/glutil.h"

struct guipass {
    ilG_stage stage;
    ilG_gui_frame *root;
};

void gui_run(ilG_stage *ptr)
{
    struct guipass *self = (struct guipass*)ptr;
    ilG_testError("Unknown");
    if (self->root) {
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        //glEnable(GL_BLEND);
        //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        self->root->rect = (ilG_gui_rect){
            .a = {0, 0, 0.f, 0.f},
            .b = {self->stage.context->width, self->stage.context->height, 0.f, 0.f}
        };
        ilG_gui_draw(self->root);
    }
    ilG_testError("gui_run");
}

struct ilG_stage *ilG_guipass(struct ilG_context *context)
{
    struct guipass *self = il_new(&ilG_guipass_type);
    self->stage.name = "GUI Renderer";
    self->stage.context = context;
    self->stage.run = gui_run;
    return &self->stage;
}

void ilG_guipass_setRoot(ilG_stage *stage, ilG_gui_frame *root)
{
    struct guipass *self = (struct guipass*)stage;
    if (self->root) {
        il_unref(self->root);
    }
    self->root = il_ref(root);
}

il_type ilG_guipass_type = {
    .typeclasses = NULL,
    .storage = {NULL},
    .constructor = NULL,
    .destructor = NULL,
    .copy = NULL,
    .name = "il.graphics.guipass",
    .size = sizeof(struct guipass),
    .parent = &ilG_stage_type
};

