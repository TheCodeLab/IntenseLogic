#include "node.h"

#include "util/log.h"

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
    if (!iface) {
        iface = il_cast(il_typeof(file), "il.asset.file");
    }
    if (!iface) {
        il_error("Parameter is not a file");
    }
    return iface->contents(file, size);
}

