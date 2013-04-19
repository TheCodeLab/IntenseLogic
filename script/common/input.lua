local ffi = require "ffi"

ffi.cdef [[

int ilI_isKeySet(int key);
int ilI_isButtonSet(int button);

void ilI_grabMouse(int mode);

]]

local input = {}

function input.isKeySet(key)
    if type(key) == "string" then
        key = string.byte(key)--key = ffi.C.il_keymap_getkey(key);
    end
    return ffi.C.ilI_isKeySet(key)
end

function input.isButtonSet(button)
    return ffi.C.ilI_isButtonSet(button)
end

function input.grabMouse(mode)
    ffi.C.ilI_grabMouse(mode)
end

return input;

