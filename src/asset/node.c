#include "node.h"

#include "util/log.h"
#include "util/assert.h"

il_base *ilA_lookup(const ilA_dir *iface, il_base *dir, const ilA_path *path)
{
    if (!iface) {
        iface = il_cast(il_typeof(dir), "il.asset.dir");
    }
    if (!iface) {
        il_error("Parameter is not a directory");
        return NULL;
    }
    return iface->lookup(dir, path);
}

il_base *ilA_create(const ilA_dir *iface, il_base *dir, const ilA_path *path, const ilA_file **res)
{
    if (!iface) {
        iface = il_cast(il_typeof(dir), "il.asset.dir");
    }
    if (!iface) {
        il_error("Parameter is not a directory");
        return NULL;
    }
    return iface->create(dir, path, res);
}

il_base *ilA_mkdir(const ilA_dir *iface, il_base *dir, const ilA_path *path, const ilA_dir **res)
{
    if (!iface) {
        iface = il_cast(il_typeof(dir), "il.asset.dir");
    }
    if (!iface) {
        il_error("Parameter is not a directory");
        return NULL;
    }
    return iface->mkdir(dir, path, res);
}

void ilA_delete(const ilA_dir *iface, il_base *dir, const ilA_path *path)
{
    if (!iface) {
        iface = il_cast(il_typeof(dir), "il.asset.dir");
    }
    if (!iface) {
        il_error("Parameter is not a directory");
        return;
    }
    iface->delete(dir, path);
}

void *ilA_contents(const ilA_file *iface, il_base *file, size_t *size)
{
    il_return_null_on_fail(file);
    if (!iface) {
        iface = il_cast(il_typeof(file), "il.asset.file");
    }
    if (!iface) {
        il_error("Parameter is not a file");
        return NULL;
    }
    return iface->contents(file, size);
}

il_base *ilA_contents_path  (const ilA_path *path, size_t *size, void **data, const ilA_file **res)
{
    const ilA_file *iface;
    il_base *base = ilA_stdiofile(path, ILA_FILE_READ, &iface);
    if (!base) {
        char *tostr = ilA_path_tochars(path);
        il_error("Unable to open file %s", tostr);
        free(tostr);
        return NULL;
    }
    if (res) {
        *res = iface;
    }
    *data = ilA_contents(iface, base, size);
    return base;
}

il_base *ilA_contents_chars (const char *str, size_t *size, void **data, const ilA_file **res)
{
    ilA_path *path = ilA_path_chars(str);
    il_base *base = ilA_contents_path(path, size, data, res);
    ilA_path_free(path);
    return base;
}

