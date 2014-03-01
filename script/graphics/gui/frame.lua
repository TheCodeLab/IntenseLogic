local ffi = require "ffi"
local base = require "common.base"

require "graphics.gui.types"
local text = require "graphics.gui.text"
local tex = require "graphics.tex"

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
typedef int (*ilG_gui_build_fn)(ilG_gui_frame *self, struct ilG_context *context);
typedef void (*ilG_gui_message_fn)(ilG_gui_frame *self, int type, il_value v);

struct ilG_gui_frame {
    il_base base;
    ilG_gui_frame *parent;
    ilG_gui_rect rect;
    ilG_gui_onClick click;
    ilG_gui_onHover hover;
    ilG_gui_draw_fn draw;
    ilG_gui_build_fn build;
    ilG_gui_message_fn message;
    struct {
        ilG_gui_frame** data;
        size_t length;
        size_t capacity;
    } children;
    struct ilG_context *context;
    _Bool complete;
};

extern il_type ilG_gui_frame_type;

void ilG_gui_frame_filler(ilG_gui_frame *self, float col[4]);
void ilG_gui_frame_image(ilG_gui_frame *self);
void ilG_gui_frame_image_setTex(ilG_gui_frame *self, struct ilG_tex *tex, int premultiplied);
ilG_gui_rect ilG_gui_frame_abs(ilG_gui_frame *self);
int ilG_gui_frame_contains(ilG_gui_frame *self, ilG_gui_coord coord);
enum ilG_gui_inputaction ilG_gui_click(ilG_gui_frame *top, int x, int y, int button);
void ilG_gui_hover(ilG_gui_frame *top, int x, int y);
void ilG_gui_draw(ilG_gui_frame *top);
void ilG_gui_addChild(ilG_gui_frame *parent, ilG_gui_frame *child);
void ilG_gui_pop(ilG_gui_frame *node);

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
    setTex = modules.graphics.ilG_gui_frame_image_setTex;
    click = modules.graphics.ilG_gui_click;
    label = function(self, layout, col, opts)
        local t = tex.image(layout:render(col, opts))
        self:image()
        self:setTex(t, 1)
        local x, y = layout:getExtents()
        self:setSize(x,y)
    end;
    setPosition = function(self, x, y, xp, yp)
        xp = xp or 0
        yp = yp or 0
        local oldx, oldy = self.rect.a.x, self.rect.a.y
        local oldxp, oldyp = self.rect.a.xp, self.rect.a.yp
        self.rect.a.x = x
        self.rect.a.y = y
        self.rect.a.xp = xp
        self.rect.a.yp = yp
        self.rect.b.x = self.rect.b.x + x - oldx
        self.rect.b.y = self.rect.b.y + y - oldy
        self.rect.b.xp = self.rect.b.xp + xp - oldxp
        self.rect.b.yp = self.rect.b.yp + yp - oldyp
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
    pop = modules.graphics.ilG_gui_pop;
    abs = modules.graphics.ilG_gui_frame_abs;
    contains = modules.graphics.ilG_gui_frame_contains;
    getChildren = function(self)
        local t = {}
        for i = 0, self.children.length-1 do
            t[#t+1] = self.children.data[i]
        end
        return t
    end
}

return modules.graphics.ilG_gui_frame_type;

