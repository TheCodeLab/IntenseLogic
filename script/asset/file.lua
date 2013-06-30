local ffi = require "ffi"

local path = require "asset.path"

ffi.cdef [[

enum ilA_file_mode {
    ILA_FILE_READ   = 1<<0,
    ILA_FILE_WRITE  = 1<<1,
    ILA_FILE_EXEC   = 1<<2
};

typedef struct ilA_file ilA_file;

il_base *ilA_stdiofile  (const ilA_path *path,  enum ilA_file_mode mode,                    const ilA_file **res);
void    *ilA_contents   (const ilA_file *iface, il_base *file, size_t *size);

]]

local file = {}

function file.load(p, mode)
    mode = mode or "r"
    local modes = {r = 1, w = 2, x = 4}
    local perm = 0
    for i = 1, #mode do
        local c = mode:sub(i,i)
        perm = bit.bor(perm, modes[c])
    end
    local f = modules.asset.ilA_stdiofile(path(p), perm, nil)
    if f == nil then
        error("Could not open file "..p)
    end
    return f
end

function file.contents(f)
    local sz = ffi.new("size_t[1]")
    local data = modules.asset.ilA_contents(nil, f, sz)
    return data, sz[0]
end

setmetatable(file, {__call = function(self,...) return file.load(...) end})

return file

