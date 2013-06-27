#ifndef ILG_GUI_TEXT_H
#define ILG_GUI_TEXT_H

#include "frame.h"

#include "util/ilstring.h"
#include "asset/node.h"

enum ilG_gui_textdir {
    ILG_GUI_DEFAULTDIR,
    ILG_GUI_LTR,
    ILG_GUI_RTL, 
    ILG_GUI_TTB,
    ILG_GUI_BTT
};

enum ilG_gui_textjustify {
    ILG_GUI_LEFTJUSTIFY,
    ILG_GUI_CENTERJUSTIFY,
    ILG_GUI_RIGHTJUSTIFY,
    ILG_GUI_HORIZMASK,
    ILG_GUI_TOPJUSTIFY = 4,
    ILG_GUI_MIDDLEJUSTIFY = 8,
    ILG_GUI_BOTTOMJUSTIFY = 12,
    ILG_GUI_VERTMASK = 7
};

typedef struct ilG_gui_textlayout ilG_gui_textlayout;

struct ilG_context;

ilG_gui_textlayout *ilG_gui_textlayout_new(struct ilG_context *ctx, const char *lang, enum ilG_gui_textdir direction, const char *script, il_base *font, const ilA_file *tc, double pt, il_string *source);
void ilG_gui_textlayout_getSize(ilG_gui_textlayout *self, unsigned *x, unsigned *y);

void ilG_gui_frame_label(ilG_gui_frame *self, ilG_gui_textlayout *layout, float col[4], enum ilG_gui_textjustify justify);

#endif

