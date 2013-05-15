--- Input functions
local ffi = require "ffi"

ffi.cdef [[

int ilI_isKeySet(int key);
int ilI_isButtonSet(int button);

void ilI_grabMouse(int mode);

]]

local input = {}

--- Returns whether the given key code is being pressed
function input.isKeySet(key)
    if type(key) == "string" then
        key = string.byte(key)--key = ffi.C.il_keymap_getkey(key);
    end
    return modules.common.ilI_isKeySet(key)
end

--- Returns whether the given mouse button is down
function input.isButtonSet(button)
    return modules.common.ilI_isButtonSet(button)
end

--- Sets whether or not the window should be grabbing the mouse and receiving relative mouse motions
function input.grabMouse(mode)
    modules.common.ilI_grabMouse(mode)
end

return input;

