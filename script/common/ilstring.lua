local ffi = require "ffi";

ffi.cdef [[

typedef struct il_string {
  size_t length;
  const char *data;
} il_string;

il_string il_CtoS(const char * s, int len);

char* strdup(const char*);

]]

local ilstring = {}

function ilstring.create(s)
    return ffi.C.il_CtoS(ffi.C.strdup(s), #s + 1); -- null terminator
end

setmetatable(ilstring, {__call = function(self, ...) return ilstring.create(...) end})

return ilstring;

