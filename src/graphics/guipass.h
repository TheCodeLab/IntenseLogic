#ifndef ILG_GUIPASS_H
#define ILG_GUIPASS_H

#include "graphics/renderer.h"

struct ilG_stage;
struct ilG_context;
struct ilG_gui_frame;

typedef struct ilG_gui ilG_gui;

extern const ilG_renderable ilG_gui_renderer;

#define ilG_gui_wrap(p) ilG_renderer_wrap(p, &ilG_gui_renderer)

ilG_gui *ilG_gui_new(struct ilG_gui_frame *root);

#endif

