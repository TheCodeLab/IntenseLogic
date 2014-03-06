local ffi = require "ffi"

local event = require "common.event"

ffi.cdef [[

int ilI_getKey(int key, int *input);
char *ilI_backend_getName(int input);

typedef struct ilI_handler {
    ilE_handler *button;
    ilE_handler *character;
    ilE_handler *mousemove;
    ilE_handler *mouseenter;
    ilE_handler *mousescroll;
} ilI_handler;

extern ilI_handler ilI_globalHandler;

int SDL_GetScancodeFromName(const char *name);

]]

keysyms = {
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

input.button        = modules.input.ilI_globalHandler.button
input.character     = modules.input.ilI_globalHandler.character
input.mousemove     = modules.input.ilI_globalHandler.mousemove
input.mouseenter    = modules.input.ilI_globalHandler.mouseenter
input.mousescroll   = modules.input.ilI_globalHandler.mousescroll

function input.get(key)
    local ret = ffi.new("int[1]")
    local str = string.upper(key:gsub(" ", "_"))
    local enum = keysyms["ILI_"..str] or 
                 keysyms["ILI_MOUSE_"..str] or
                 keysyms["ILI_JOY_"..str] or
                 ffi.C.SDL_GetScancodeFromName(key)
    return modules.input.ilI_getKey(enum, ret) == 1, ret[0]
end

return input

