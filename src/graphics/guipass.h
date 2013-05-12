#ifndef ILG_GUIPASS_H
#define ILG_GUIPASS_H

#include "common/base.h"

struct ilG_stage;
struct ilG_context;
struct ilG_gui_frame;

struct ilG_stage *ilG_guipass(struct ilG_context *context);
void ilG_guipass_setRoot(struct ilG_stage *self, struct ilG_gui_frame *root);

extern il_type ilG_guipass_type;

#endif

