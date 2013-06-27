#include "text.h"

#include <GL/glew.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ftadvanc.h>
#include <freetype/ftsnames.h>
#include <freetype/tttables.h>

#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include <harfbuzz/hb-icu.h>

#include <cairo/cairo.h>
#include <cairo/cairo-ft.h>

#include "util/assert.h"
#include "graphics/context.h"

struct ilG_gui_textlayout {
    char *lang, *script;
    il_base *font;
    enum ilG_gui_textdir direction;
    double pt;
    il_string *source;
    hb_buffer_t *buf;
    hb_font_t *hb_ft_font;
    FT_Face ft_face;
    unsigned int glyph_count;
    hb_glyph_info_t *glyph_info;
    hb_glyph_position_t *glyph_pos;
};

struct text_globalctx {
    FT_Library library;
};

ilG_gui_textlayout *ilG_gui_textlayout_new(struct ilG_context *ctx, const char *lang, enum ilG_gui_textdir direction, const char *script, il_base *font, const ilA_file *tc, double pt, il_string *source)
{
    struct text_globalctx *gctx = il_base_get(&ctx->base, "il.graphics.gui.text.ctx", NULL, NULL);
    if (!gctx) {
        gctx = calloc(1, sizeof(struct text_globalctx));

        il_return_null_on_fail(!FT_Init_FreeType(&gctx->library));

        il_base_set(&ctx->base, "il.graphics.gui.text.ctx", gctx, sizeof(struct text_globalctx), IL_VOID);
    }

    ilG_gui_textlayout *l = calloc(1, sizeof(ilG_gui_textlayout));
    l->lang = strdup(lang);
    l->script = strdup(script);
    l->font = il_ref(font);
    l->direction = direction;
    l->pt = pt;
    l->source = il_string_ref(source);
    size_t size;
    void *data = ilA_contents(tc, font, &size);
    il_return_null_on_fail(!FT_New_Memory_Face(gctx->library, data, size, 0, &l->ft_face));
    il_return_null_on_fail(!FT_Set_Char_Size(l->ft_face, 0, pt, 0, 0));
    l->hb_ft_font = hb_ft_font_create(l->ft_face, NULL);

    l->buf = hb_buffer_create();
    hb_buffer_set_unicode_funcs(l->buf, hb_icu_get_unicode_funcs());
    hb_buffer_set_direction(l->buf, direction + 4); // Hacky solution
    hb_buffer_set_script(l->buf, hb_script_from_string(script, -1));
    hb_buffer_set_language(l->buf, hb_language_from_string(lang, -1));
    hb_buffer_add_utf8(l->buf, source->data, source->length, 0, source->length);
    hb_shape(l->hb_ft_font, l->buf, NULL, 0);
    l->glyph_info = hb_buffer_get_glyph_infos(l->buf, &l->glyph_count);
    l->glyph_pos = hb_buffer_get_glyph_positions(l->buf, &l->glyph_count);
    return l;
}

void ilG_gui_textlayout_getSize(ilG_gui_textlayout *self, unsigned *x, unsigned *y)
{
    unsigned w = 0, h = 0, i;
    for (i = 0; i < self->glyph_count; i++) {
        w += self->glyph_pos[i].x_advance;
        h += self->glyph_pos[i].y_advance;
    }
    if (x) {
        *x = w/64;
    }
    if (y) {
        *y = h/64;
    }
}

struct text_ctx {
    cairo_surface_t *surface;
    cairo_t *cr;
    cairo_font_face_t *cairo_ft_face;
    hb_face_t *hb_ft_face;
    float col[4];
    enum ilG_gui_textjustify horiz, vert;
};

void ilG_gui_frame_label(ilG_gui_frame *self, ilG_gui_textlayout *layout, float col[4], enum ilG_gui_textjustify justify)
{
    struct text_ctx *ctx = il_base_get(&self->base, "il.graphics.gui.text.ctx", NULL, NULL);
    if (!ctx) {
        ctx = calloc(1, sizeof(struct text_ctx));
        
        ctx->cairo_ft_face = cairo_ft_font_face_create_for_ft_face(layout->ft_face, 0);
        ctx->hb_ft_face = hb_ft_face_create(layout->ft_face, NULL);

        ilG_gui_rect rect = ilG_gui_frame_abs(self);
        int w = rect.b.x - rect.a.x, h = rect.b.y - rect.a.y;
        ctx->surface = cairo_image_surface_create(CAIRO_FORMAT_A8, w, h);
        ctx->cr = cairo_create(ctx->surface);

        unsigned tw, th;
        ilG_gui_textlayout_getSize(layout, &tw, &th);
        unsigned x = 0, y = 0;
        switch(justify & ILG_GUI_HORIZMASK) {
            case ILG_GUI_LEFTJUSTIFY:
            x = 0;
            break;
            case ILG_GUI_CENTERJUSTIFY:
            x = w/2 - tw/2;
            break;
            case ILG_GUI_RIGHTJUSTIFY:
            x = w - tw;
            break;
            default:
            break;
        }
        switch(justify & ILG_GUI_VERTMASK) {
            case ILG_GUI_TOPJUSTIFY:
            y = 0;
            break;
            case ILG_GUI_MIDDLEJUSTIFY:
            y = h/2 - th/2;
            break;
            case ILG_GUI_BOTTOMJUSTIFY:
            y = h - th;
            break;
            default:
            break;
        }

        x *= 64;
        y *= 64;
        cairo_glyph_t cairo_glyphs[sizeof(cairo_glyph_t) * layout->glyph_count];
        unsigned i;
        for (i = 0; i < layout->glyph_count; i++) {
            cairo_glyphs[i].index = layout->glyph_info[i].codepoint;
            x += layout->glyph_pos[i].x_advance;
            y += layout->glyph_pos[i].y_advance;
            cairo_glyphs[i].x = x/64;
            cairo_glyphs[i].y = y/64;
        }

        cairo_set_source_rgba(ctx->cr, col[0], col[1], col[2], col[3]);
        cairo_set_font_face(ctx->cr, ctx->cairo_ft_face);
        cairo_set_font_size(ctx->cr, layout->pt);
        cairo_show_glyphs(ctx->cr, cairo_glyphs, layout->glyph_count);

        il_base_set(&self->base, "il.graphics.gui.text.ctx", ctx, sizeof(struct text_ctx), IL_VOID);
    }
}

