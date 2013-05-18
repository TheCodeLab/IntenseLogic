local ffi = require "ffi"

require "util.ilstring"

ffi.cdef [[

typedef struct ilA_path ilA_path;

ilA_path* ilA_path_string(il_string *path);
ilA_path* ilA_path_chars(const char *path);
ilA_path* ilA_path_cwd();
ilA_path* ilA_path_copy(const ilA_path *self);
void ilA_path_free(ilA_path* self);
il_string *ilA_path_tostr(const ilA_path* self);
char *ilA_path_tochars(const ilA_path* self);
int ilA_path_cmp(const ilA_path* a, const ilA_path* b);
ilA_path* ilA_path_concat(const ilA_path* a, const ilA_path* b);
ilA_path* ilA_path_relativeTo(const ilA_path* a, const ilA_path* b);

]]

local path = {}

path.__gc = modules.asset.ilA_path_free

function path:__call(p)
    if ffi.istype("il_string", p) then
        return modules.asset.ilA_path_string(p)
    elseif type(p) == "string" then
        return modules.asset.ilA_path_chars(p)
    elseif ffi.istype("ilA_path", p) then
        return p
    else
        error("Expected string")
    end
end

path.cwd = modules.asset.ilA_path_cwd
path.copy = modules.asset.ilA_path_copy
path.__tostring = function(s)
    return tostring(modules.asset.ilA_path_tostr(s))
end
function path.__eq(a, b)
    return modules.asset.ilA_path_cmp(a,b) == 0
end
function path.__lt(a, b)
    return modules.asset.ilA_path_cmp(a,b) < 0
end
function path.__concat(a,b)
    return modules.asset.ilA_path_concat(a, path(b))
end
function path.__sub(a,b)
    return modules.asset.ilA_path_relativeTo(a, path(b))
end

ffi.metatype("ilA_path", path)
setmetatable(path, path)
return path

