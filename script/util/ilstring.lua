local ffi = require "ffi";

ffi.cdef [[

typedef struct il_string {
    size_t length, capacity, canary;
    char *data, *start;
    int *refs;
} il_string;

const il_string *il_string_static(const char *s);
il_string *il_string_new(const char *s, int len);
il_string *il_string_copy(const il_string *s);
char *il_string_cstring(const il_string *s, size_t *len);
int il_string_verify(const il_string *s);
il_string *il_string_ref(void* s);
void il_string_unref(void* s);
il_string *il_string_sub(il_string *s, int p1, int p2);
int il_string_cmp(const il_string *a, const il_string *b);
int il_string_byte(const il_string *s, int pos);
il_string *il_string_format(const char *fmt, ...);

]]

local ilstring = {}

function ilstring.create(s)
    return modules.util.il_string_new(s, #s)
end

setmetatable(ilstring, {__call = function(self, ...) return ilstring.create(...) end})

return ilstring;

