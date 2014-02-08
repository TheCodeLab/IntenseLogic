#ifndef ILG_GUIPASS_H
#define ILG_GUIPASS_H

#include "graphics/stage.h"

struct ilG_stage;
struct ilG_context;
struct ilG_gui_frame;

typedef struct ilG_gui ilG_gui;

extern const ilG_stagable ilG_gui_stage;

ilG_gui *ilG_gui_new(struct ilG_context *context, struct ilG_gui_frame *root);

#endif

