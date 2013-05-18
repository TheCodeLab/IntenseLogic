local ffi = require "ffi"

require "asset.path"

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



