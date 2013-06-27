local ffi = require "ffi"

local event = require "common.event"

ffi.cdef [[

enum ilI_mod {
    ILI_MOD_SHIFT   = 0x1,
    ILI_MOD_CONTROL = 0x2,
    ILI_MOD_ALT     = 0x4,
    ILI_MOD_SUPER   = 0x8
};

int ilI_getKey(int key, int *input);
char *ilI_backend_getName(int input);

typedef struct ilI_buttonevent {
    int button;
    int scancode;
    int device;
    int action;
    enum ilI_mod mods;
} ilI_buttonevent;

]]

keysyms = {
    ILI_KEY_WORLD_1         = 161,
    ILI_KEY_WORLD_2         = 162,
    ILI_KEY_ESCAPE          = 256,
    ILI_KEY_ENTER           = 257,
    ILI_KEY_TAB             = 258,
    ILI_KEY_BACKSPACE       = 259,
    ILI_KEY_INSERT          = 260,
    ILI_KEY_DELETE          = 261,
    ILI_KEY_RIGHT           = 262,
    ILI_KEY_LEFT            = 263,
    ILI_KEY_DOWN            = 264,
    ILI_KEY_UP              = 265,
    ILI_KEY_PAGE_UP         = 266,
    ILI_KEY_PAGE_DOWN       = 267,
    ILI_KEY_HOME            = 268,
    ILI_KEY_END             = 269,
    ILI_KEY_CAPS_LOCK       = 280,
    ILI_KEY_SCROLL_LOCK     = 281,
    ILI_KEY_NUM_LOCK        = 282,
    ILI_KEY_PRINT_SCREEN    = 283,
    ILI_KEY_PAUSE           = 284,
    ILI_KEY_F1              = 290,
    ILI_KEY_F2              = 291,
    ILI_KEY_F3              = 292,
    ILI_KEY_F4              = 293,
    ILI_KEY_F5              = 294,
    ILI_KEY_F6              = 295,
    ILI_KEY_F7              = 296,
    ILI_KEY_F8              = 297,
    ILI_KEY_F9              = 298,
    ILI_KEY_F10             = 299,
    ILI_KEY_F11             = 300,
    ILI_KEY_F12             = 301,
    ILI_KEY_F13             = 302,
    ILI_KEY_F14             = 303,
    ILI_KEY_F15             = 304,
    ILI_KEY_F16             = 305,
    ILI_KEY_F17             = 306,
    ILI_KEY_F18             = 307,
    ILI_KEY_F19             = 308,
    ILI_KEY_F20             = 309,
    ILI_KEY_F21             = 310,
    ILI_KEY_F22             = 311,
    ILI_KEY_F23             = 312,
    ILI_KEY_F24             = 313,
    ILI_KEY_F25             = 314,
    ILI_KEY_KP_0            = 320,
    ILI_KEY_KP_1            = 321,
    ILI_KEY_KP_2            = 322,
    ILI_KEY_KP_3            = 323,
    ILI_KEY_KP_4            = 324,
    ILI_KEY_KP_5            = 325,
    ILI_KEY_KP_6            = 326,
    ILI_KEY_KP_7            = 327,
    ILI_KEY_KP_8            = 328,
    ILI_KEY_KP_9            = 329,
    ILI_KEY_KP_DECIMAL      = 330,
    ILI_KEY_KP_DIVIDE       = 331,
    ILI_KEY_KP_MULTIPLY     = 332,
    ILI_KEY_KP_SUBTRACT     = 333,
    ILI_KEY_KP_ADD          = 334,
    ILI_KEY_KP_ENTER        = 335,
    ILI_KEY_KP_EQUAL        = 336,
    ILI_KEY_LEFT_SHIFT      = 340,
    ILI_KEY_LEFT_CONTROL    = 341,
    ILI_KEY_LEFT_ALT        = 342,
    ILI_KEY_LEFT_SUPER      = 343,
    ILI_KEY_RIGHT_SHIFT     = 344,
    ILI_KEY_RIGHT_CONTROL   = 345,
    ILI_KEY_RIGHT_ALT       = 346,
    ILI_KEY_RIGHT_SUPER     = 347,
    ILI_KEY_MENU            = 348,
    ILI_MOUSE_LEFT          = 512,
    ILI_MOUSE_RIGHT         = 513,
    ILI_MOUSE_MIDDLE        = 514,
    ILI_MOUSE_4             = 515,
    ILI_MOUSE_5             = 516,
    ILI_MOUSE_6             = 517,
    ILI_MOUSE_7             = 518,
    ILI_MOUSE_8             = 519,
    ILI_JOY_1               = 768,
    ILI_JOY_2               = 769,
    ILI_JOY_3               = 770,
    ILI_JOY_4               = 771,
    ILI_JOY_5               = 772,
    ILI_JOY_6               = 773,
    ILI_JOY_7               = 774,
    ILI_JOY_8               = 775,
    ILI_JOY_9               = 776,
    ILI_JOY_10              = 777,
    ILI_JOY_11              = 778,
    ILI_JOY_12              = 779,
    ILI_JOY_13              = 780,
    ILI_JOY_14              = 781,
    ILI_JOY_15              = 782,
    ILI_JOY_16              = 783,
};

local input = {}

function input.buttonUnpacker(size, data)
    local ev = ffi.cast("ilI_buttonevent*", data)
    local button
    for k, v in pairs(keysyms) do
        if v == ev.button then
            button = string.lower(k:sub(5, -1):gsub("_", " "))
            break
        end
    end
    local mods = ""
    for k,v in pairs {S=1, C=2, A=4, W=8} do
        if bit.band(v, ev.mods) ~= 0 then
            mods = mods .. k
        end
    end
    return button, ev.scancode, ev.device, ev.action == 1, mods
end

function input.inputUnpackers(reg)
    event.setUnpacker(reg, "input.button", input.buttonUnpacker)
    event.setUnpacker(reg, "input.mousemove", event.arrayUnpacker("int", 4))
    event.setUnpacker(reg, "input.mouseenter", event.typeUnpacker("int"))
    event.setUnpacker(reg, "input.mousescroll", event.arrayUnpacker("float", 2))
    event.setUnpacker(reg, "input.character", event.typeUnpacker("unsigned int"))
end

input.inputUnpackers(event.registry)

function input.get(key)
    local ret = ffi.new("int[1]")
    local str = string.upper(key:gsub(" ", "_"))
    local enum = keysyms["ILI_"..str] or 
                 keysyms["ILI_KEY_"..str] or
                 keysyms["ILI_MOUSE_"..str] or
                 keysyms["ILI_JOY_"..str] or
                 (#key == 1 and string.byte(key) or error "Invalid key specification")
    return modules.input.ilI_getKey(enum, ret) == 1, ret[0]
end

return input

