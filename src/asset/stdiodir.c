#include "node.h"

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>

struct stdiodir {
    il_base base;
    ilA_path *path;
};

static void stdiodir_des(void *ptr)
{
    struct stdiodir *self = ptr;
    ilA_path_free(self->path);
}

static il_base* stdiodir_lookup(void* ptr, const ilA_path *path)
{
    struct stdiodir *self = ptr;
    struct stat s;
    ilA_path *full;
    char *buf;
    il_base *node;
   
    full = ilA_path_concat(self->path, path);
    buf = ilA_path_tochars(full);
    if (stat(buf, &s) != 0) {
        return NULL;
    }
    free(buf);
    if (S_ISDIR(s.st_mode)) {
        node = ilA_stdiodir(full, NULL);
    } else {
        node = ilA_stdiofile(full, ILA_FILE_READ, NULL);
    }
    ilA_path_free(full);
    return node;
}

// TODO: implement create, mkdir, and delete

il_type il_stdiodir_type = {
    .typeclasses = NULL,
    .storage = {NULL},
    .constructor = NULL,
    .destructor = stdiodir_des,
    .copy = NULL,
    .name = "il.asset.stdiodir",
    .size = sizeof(struct stdiodir),
    .parent = NULL
};

ilA_dir il_stdiodir_dir = {
    .name = "il.asset.dir",
    .lookup = stdiodir_lookup
};

void ilA_stdiodir_init()
{
    il_impl(&il_stdiodir_type, &il_stdiodir_dir);
}

il_base *ilA_stdiodir(const ilA_path *path, const ilA_dir **res)
{
    struct stdiodir *d = il_new(&il_stdiodir_type);
    d->path = ilA_path_copy(path);
    if (res) {
        *res = &il_stdiodir_dir;
    }
    return &d->base;
}

