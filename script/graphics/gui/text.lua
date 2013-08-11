local ffi = require "ffi"

local base = require "common.base"
local ilstring = require "util.ilstring"
local file = require "asset.file"

ffi.cdef [[

enum ilG_gui_textdir {
    ILG_GUI_DEFAULTDIR,
    ILG_GUI_LTR,
    ILG_GUI_RTL, 
    ILG_GUI_TTB,
    ILG_GUI_BTT
};

enum ilG_gui_textjustify {
    ILG_GUI_LEFTJUSTIFY = 1,
    ILG_GUI_CENTERJUSTIFY,
    ILG_GUI_RIGHTJUSTIFY,
    ILG_GUI_HORIZMASK = 3,
    ILG_GUI_TOPJUSTIFY = 4,
    ILG_GUI_MIDDLEJUSTIFY = 8,
    ILG_GUI_BOTTOMJUSTIFY = 12,
    ILG_GUI_VERTMASK = 12
};

enum ilG_gui_textoptions {
    ILG_GUI_BOLD = 1,
    ILG_GUI_ITALIC = 2,
    ILG_GUI_UNDERLINE = 4,
    ILG_GUI_STRIKETHROUGH = 8
};

typedef struct ilG_gui_textlayout ilG_gui_textlayout;
extern il_type ilG_gui_textlayout_type;

ilG_gui_textlayout *ilG_gui_textlayout_new(struct ilG_context *ctx, const char *lang, enum ilG_gui_textdir direction, const char *script, il_base *font, const ilA_file *tc, double pt, il_string *source);
void ilG_gui_textlayout_getExtents(ilG_gui_textlayout *self, unsigned *rx, unsigned *ry, unsigned *bx, unsigned *by, unsigned *ax, unsigned *ay);
ilA_img *ilG_gui_textlayout_render(ilG_gui_textlayout *self, float col[4], enum ilG_gui_textoptions opts);

]]

base.wrap "il.graphics.gui.textlayout" {
    struct = "ilG_gui_textlayout";
    __call = function(self, ctx, lang, dir, script, font, pt, source)
        source = type(source) == "string" and ilstring(source) or source
        if type(font) == "string" then
            font = file.load(font)
        end
        if not font then
            error "Could not load font"
        end
        local tl = modules.graphics.ilG_gui_textlayout_new(ctx, lang, "ILG_GUI_"..string.upper(dir), script, font, nil, pt, source)
        ffi.gc(tl, modules.common.il_unref)
        return tl
    end;
    getExtents = function(self)
        local sz = ffi.new("unsigned[6]")
        modules.graphics.ilG_gui_textlayout_getExtents(self, sz, sz+1, sz+2, sz+3, sz+4, sz+5)
        return sz[0], sz[1], sz[2], sz[3], sz[4], sz[5]
    end;
    render = function(self, col, opts)
        local arr = ffi.new("float[4]", col)
        local img = modules.graphics.ilG_gui_textlayout_render(self, arr, 0)
        ffi.gc(img, modules.asset.ilA_img_free)
        return img
    end;
}

return modules.graphics.ilG_gui_textlayout_type

