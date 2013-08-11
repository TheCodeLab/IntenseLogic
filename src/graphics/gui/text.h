#ifndef ILG_GUI_TEXT_H
#define ILG_GUI_TEXT_H

#include "frame.h"

#include "util/ilstring.h"
#include "asset/node.h"
#include "asset/image.h"

enum ilG_gui_textdir {
    ILG_GUI_DEFAULTDIR,
    ILG_GUI_LTR,
    ILG_GUI_RTL, 
    ILG_GUI_TTB,
    ILG_GUI_BTT
};

/*enum ilG_gui_textjustify {
    ILG_GUI_LEFTJUSTIFY = 1,
    ILG_GUI_CENTERJUSTIFY,
    ILG_GUI_RIGHTJUSTIFY,
    ILG_GUI_HORIZMASK = 3,
    ILG_GUI_TOPJUSTIFY = 4,
    ILG_GUI_MIDDLEJUSTIFY = 8,
    ILG_GUI_BOTTOMJUSTIFY = 12,
    ILG_GUI_VERTMASK = 12
};*/

enum ilG_gui_textoptions {
    ILG_GUI_BOLD = 1,
    ILG_GUI_ITALIC = 2,
    ILG_GUI_UNDERLINE = 4,
    ILG_GUI_STRIKETHROUGH = 8
};

typedef struct ilG_gui_textlayout ilG_gui_textlayout;
extern il_type ilG_gui_textlayout_type;

struct ilG_context;

ilG_gui_textlayout *ilG_gui_textlayout_new(struct ilG_context *ctx, const char *lang, enum ilG_gui_textdir direction, const char *script, il_base *font, const ilA_file *tc, double pt, il_string *source);
void ilG_gui_textlayout_getExtents(ilG_gui_textlayout *self, unsigned *rx, unsigned *ry, int *bx, int *by, int *ax, int *ay);

ilA_img *ilG_gui_textlayout_render(ilG_gui_textlayout *self, float col[4], enum ilG_gui_textoptions opts);

#endif

