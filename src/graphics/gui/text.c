#include "text.h"

#include <GL/glew.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_ADVANCES_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_TABLES_H

#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include <harfbuzz/hb-icu.h>

#include <cairo/cairo.h>
#include <cairo/cairo-ft.h>

#include "util/ilassert.h"
#include "graphics/context.h"

struct ilG_gui_textlayout {
    il_base base;
    ilG_context *context;
    // layout
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
    // rendering
    cairo_surface_t *surface;
    cairo_t *cr;
    cairo_font_face_t *cairo_ft_face;
    hb_face_t *hb_ft_face;
    float col[4];
    cairo_glyph_t *cairo_glyphs;
    cairo_text_extents_t extents;
    int have_size;
};

static void text_des(void *obj)
{
    ilG_gui_textlayout *self = obj;
    il_unref(self->font);
    free(self->lang);
    free(self->script);
    il_string_unref(self->source);

    hb_buffer_destroy(self->buf);
    hb_face_destroy(self->hb_ft_face);
    hb_font_destroy(self->hb_ft_font);

    cairo_destroy(self->cr);
    cairo_surface_destroy(self->surface);
    cairo_font_face_destroy(self->cairo_ft_face);

    FT_Done_Face(self->ft_face);
}

il_type ilG_gui_textlayout_type = {
    .typeclasses = NULL,
    .storage = {NULL},
    .constructor = NULL,
    .destructor = text_des,
    .copy = NULL, 
    .name = "il.graphics.gui.textlayout",
    .size = sizeof(ilG_gui_textlayout),
    .parent = NULL
};

struct text_globalctx {
    FT_Library library;
};

ilG_gui_textlayout *ilG_gui_textlayout_new(ilG_context *ctx, const char *lang, enum ilG_gui_textdir direction, const char *script, il_base *font, const ilA_file *tc, double pt, il_string *source)
{
    il_return_null_on_fail(ctx && lang && script && font && source);
    struct text_globalctx *gctx = il_table_getsp(&ctx->base.storage, "gui.text.ctx");

    if (!gctx) {
        gctx = calloc(1, sizeof(struct text_globalctx));
        il_return_null_on_fail(!FT_Init_FreeType(&gctx->library));
        il_table_setsp(&ctx->base.storage, "gui.text.ctx", il_opaque(gctx, free));
    }
    ilG_gui_textlayout *l = il_new(&ilG_gui_textlayout_type);
    l->context = ctx;
    l->lang = strdup(lang);
    l->script = strdup(script);
    l->font = il_ref(font);
    l->direction = direction;
    l->pt = pt;
    l->source = il_string_ref(source);
    size_t size;
    void *data = ilA_contents(tc, font, &size);
    if (!data) {
        il_error("Could not open font");
        return NULL;
    }
    il_return_null_on_fail(!FT_New_Memory_Face(gctx->library, data, size, 0, &l->ft_face));
    il_return_null_on_fail(!FT_Set_Char_Size(l->ft_face, 0, pt * 64, 0, 0));
    l->hb_ft_font = hb_ft_font_create(l->ft_face, NULL);

    l->buf = hb_buffer_create();
    hb_buffer_set_unicode_funcs(l->buf, hb_icu_get_unicode_funcs());
    if (direction == ILG_GUI_DEFAULTDIR) { // TODO: find out how to determine this based on script
        direction = ILG_GUI_LTR;
    }
    hb_buffer_set_direction(l->buf, direction + 3); // Hacky solution
    hb_buffer_set_script(l->buf, hb_script_from_string(script, -1));
    hb_buffer_set_language(l->buf, hb_language_from_string(lang, -1));
    hb_buffer_add_utf8(l->buf, source->data, source->length, 0, source->length);
    hb_shape(l->hb_ft_font, l->buf, NULL, 0);
    l->glyph_info = hb_buffer_get_glyph_infos(l->buf, &l->glyph_count);
    l->glyph_pos = hb_buffer_get_glyph_positions(l->buf, &l->glyph_count);
    return l;
}

void ilG_gui_textlayout_getExtents(ilG_gui_textlayout *self, unsigned *rx, unsigned *ry, int *bx, int *by, int *ax, int *ay)
{
    if (!self->have_size) {
        self->have_size = 1;
        cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 0, 0);
        cairo_t *cr = cairo_create(surface);

        il_return_on_fail(self->cairo_glyphs);
        cairo_set_font_face(cr, self->cairo_ft_face);
        cairo_set_font_size(cr, self->pt);
        cairo_glyph_extents(cr, self->cairo_glyphs, self->glyph_count, &self->extents);
        cairo_surface_destroy(surface);
        cairo_destroy(cr);
    }
    if (rx) {
        *rx = self->extents.width;
    }
    if (ry) {
        *ry = self->extents.height + 1; // TODO: ????
    }
    if (bx) {
        *bx = self->extents.x_bearing;
    }
    if (by) {
        *by = self->extents.y_bearing;
    }
    if (ax) {
        *ax = self->extents.x_advance;
    }
    if (ay) {
        *ay = self->extents.y_advance;
    }
}

ilA_img *ilG_gui_textlayout_render(ilG_gui_textlayout *self, float col[4], enum ilG_gui_textoptions opts)
{
    (void)opts;
    unsigned w = 0, h = 0;
    unsigned x = 0, y = 0;
    int bx = 0, by = 0;
    int scale = 64;
    unsigned i;

    self->cairo_ft_face = cairo_ft_font_face_create_for_ft_face(self->ft_face, 0);
    self->hb_ft_face = hb_ft_face_create(self->ft_face, NULL);

    x *= scale;
    y *= scale;
    self->cairo_glyphs = calloc(self->glyph_count, sizeof(cairo_glyph_t));
    for (i = 0; i < self->glyph_count; i++) {
        self->cairo_glyphs[i].index = self->glyph_info[i].codepoint;
        self->cairo_glyphs[i].x = (x + self->glyph_pos[i].x_offset)/scale;
        self->cairo_glyphs[i].y = (y + self->glyph_pos[i].y_offset)/scale;
        x += self->glyph_pos[i].x_advance;
        y += self->glyph_pos[i].y_advance;
    }

    ilG_gui_textlayout_getExtents(self, &w, &h, &bx, &by, NULL, NULL);
    self->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
    self->cr = cairo_create(self->surface);

    cairo_translate(self->cr, -bx, 1-by);
    cairo_set_source_rgba(self->cr, col[0], col[1], col[2], col[3]);
    cairo_set_font_face(self->cr, self->cairo_ft_face);
    cairo_set_font_size(self->cr, self->pt);
    cairo_show_glyphs(self->cr, self->cairo_glyphs, self->glyph_count);
    cairo_surface_flush(self->surface);

    ilA_img *preswizzle = ilA_img_fromdata(cairo_image_surface_get_data(self->surface), w, h, 8, ILA_IMG_RGBA);
    /*   R G B A | 0x
     * B 0 0 1 0 | 2
     * G 0 1 0 0 | 4
     * R 1 0 0 0 | 8
     * A 0 0 0 1 | 1
    */
    ilA_img *img = ilA_img_bgra_to_rgba(preswizzle);//ilA_img_swizzle(preswizzle, 0x2481);
    ilA_img_free(preswizzle);
    return img;
}

