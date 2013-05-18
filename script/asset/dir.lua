local ffi = require "ffi"

require "asset.path"
require "asset.file"

ffi.cdef [[

typedef struct ilA_dir ilA_dir;

il_base *ilA_stdiodir   (const ilA_path *path,  const ilA_dir **res);
il_base *ilA_union      (const ilA_dir  *ai,    const ilA_dir *bi, il_base *a, il_base *b,  const ilA_dir **res);
il_base *ilA_prefix     (const ilA_dir  *iface, il_base *dir, const ilA_path *path,         const ilA_dir **res);
il_base *ilA_lookup     (const ilA_dir  *iface, il_base *dir, const ilA_path *path);
il_base *ilA_create     (const ilA_dir  *iface, il_base *dir, const ilA_path *path,         const ilA_file **res);
il_base *ilA_mkdir      (const ilA_dir  *iface, il_base *dir, const ilA_path *path,         const ilA_dir **res);
//void     ilA_delete     (const ilA_dir  *iface, il_base *dir, const ilA_path *path);

]]



