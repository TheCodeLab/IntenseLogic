local ffi = require "ffi"
local base = require "common.base"

require "graphics.gui.types"
require "graphics.gui.text"

ffi.cdef [[

typedef struct ilG_gui_frame ilG_gui_frame;

enum ilG_gui_inputaction {
    ILG_GUI_UNHANDLED,
    ILG_GUI_PASSTHROUGH,
    ILG_GUI_OVERRIDE
};

typedef enum ilG_gui_inputaction (*ilG_gui_onClick)(ilG_gui_frame *self, int x, int y, int button);
typedef enum ilG_gui_inputaction (*ilG_gui_onHover)(ilG_gui_frame *self, int x, int y);
typedef void (*ilG_gui_draw_fn)(ilG_gui_frame *self, ilG_gui_rect where);

struct ilG_gui_frame {
    il_base base;
    ilG_gui_frame *parent;
    ilG_gui_rect rect;
    ilG_gui_onClick click;
    ilG_gui_onHover hover;
    ilG_gui_draw_fn draw;
    struct {
        ilG_gui_frame** data;
        size_t length;
        size_t capacity;
    } children;
    struct ilG_context *context;
};

extern il_type ilG_gui_frame_type;

void ilG_gui_frame_filler(ilG_gui_frame *self, float col[4]);
void ilG_gui_frame_image(ilG_gui_frame *self, struct ilG_texture *tex);
ilG_gui_rect ilG_gui_frame_abs(ilG_gui_frame *self);
int ilG_gui_frame_contains(ilG_gui_frame *self, ilG_gui_coord coord);
enum ilG_gui_inputaction ilG_gui_click(ilG_gui_frame *top, int x, int y, int button);
void ilG_gui_hover(ilG_gui_frame *top, int x, int y);
void ilG_gui_draw(ilG_gui_frame *top);
void ilG_gui_addChild(ilG_gui_frame *parent, ilG_gui_frame *child);

void ilG_gui_frame_label(ilG_gui_frame *self, ilG_gui_textlayout *layout, float col[4], enum ilG_gui_textjustify justify);

]]

base.wrap "il.graphics.gui.frame" {
    struct = "ilG_gui_frame";
    filler = function(self, col)
        local v = ffi.new("float[4]")
        if type(col) == "table" then
            v[0] = col[1]
            v[1] = col[2]
            v[2] = col[3]
            v[3] = col[4]
        end
        return modules.graphics.ilG_gui_frame_filler(self, v)
    end;
    image = modules.graphics.ilG_gui_frame_image;
    label = function(self, layout, col, justify)
        local arr = ffi.new("float[4]", col)
        local mask = 0
        for w in justify:gmatch("%w+") do
            mask = bit.bor(mask, tonumber(ffi.cast("enum ilG_gui_textjustify", "ILG_GUI_"..w:upper().."JUSTIFY")))
            print(w, mask)
        end
        modules.graphics.ilG_gui_frame_label(self, layout, arr, mask)
    end;
    setPosition = function(self, x, y, xp, yp)
        xp = xp or 0
        yp = yp or 0
        self.rect.a.x = x
        self.rect.a.y = y
        self.rect.a.xp = xp
        self.rect.a.yp = yp
    end;
    setSize = function(self, w, h, wp, hp)
        wp = wp or 0
        hp = hp or 0
        self.rect.b.x = self.rect.a.x + w
        self.rect.b.y = self.rect.a.y + h
        self.rect.b.xp = self.rect.a.xp + wp
        self.rect.b.yp = self.rect.a.yp + hp
    end;
    addChild = modules.graphics.ilG_gui_addChild;
    abs = modules.graphics.ilG_gui_frame_abs;
    contains = modules.graphics.ilG_gui_frame_contains;
}

return modules.graphics.ilG_gui_frame_type;

