#ifndef ILG_FRAME_H
#define ILG_FRAME_H

#include "common/base.h"
#include "graphics/gui/types.h"
#include "util/array.h"
#include "graphics/renderer.h"

struct ilG_context;
struct ilG_tex;

typedef struct ilG_gui_frame ilG_gui_frame;

enum ilG_gui_inputaction {
    ILG_GUI_UNHANDLED,
    ILG_GUI_PASSTHROUGH,
    ILG_GUI_OVERRIDE
};

// TODO: replace with a single callback which takes an input event structure
typedef enum ilG_gui_inputaction (*ilG_gui_onClick)(ilG_gui_frame *self, int x, int y, int button);
typedef enum ilG_gui_inputaction (*ilG_gui_onHover)(ilG_gui_frame *self, int x, int y);
typedef void (*ilG_gui_draw_fn)(ilG_gui_frame *self, ilG_gui_rect where);
typedef int (*ilG_gui_build_fn)(ilG_gui_frame *self, struct ilG_context *context);

struct ilG_gui_frame {
    il_base base;
    ilG_gui_frame *parent;
    ilG_gui_rect rect;
    ilG_gui_onClick click;
    ilG_gui_onHover hover;
    ilG_gui_draw_fn draw;
    ilG_gui_build_fn build;
    IL_ARRAY(ilG_gui_frame*,) children;
    struct ilG_context *context;
    _Bool complete;
};

extern il_type ilG_gui_frame_type;
extern const ilG_renderable ilG_gui_frame_renderer;

#define ilG_gui_frame_wrap(p) ilG_renderer_wrap(p, &ilG_gui_frame_renderer)

void ilG_gui_frame_filler(ilG_gui_frame *self, float col[4]);
void ilG_gui_frame_image(ilG_gui_frame *self, struct ilG_tex tex, int premultiplied);
ilG_gui_rect ilG_gui_frame_abs(ilG_gui_frame *self);
int ilG_gui_frame_contains(ilG_gui_frame *self, ilG_gui_coord coord);
enum ilG_gui_inputaction ilG_gui_click(ilG_gui_frame *top, int x, int y, int button);
void ilG_gui_hover(ilG_gui_frame *top, int x, int y);
void ilG_gui_draw(ilG_gui_frame *top);
void ilG_gui_addChild(ilG_gui_frame *parent, ilG_gui_frame *child);
void ilG_gui_pop(ilG_gui_frame *node);

#endif

