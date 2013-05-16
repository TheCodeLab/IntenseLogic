#include "node.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

struct stdiofile {
    il_base base;
    int fd;
    void *contents;
    size_t size;
    char *path;
    enum ilA_file_mode mode;
};

static void stdiofile_des(void *ptr)
{
    struct stdiofile *self = ptr;
    munmap(self->contents, self->size);
    close(self->fd);
    free(self->path);
}

static enum ilA_file_mode stdiofile_mode(void *ptr)
{
    struct stdiofile *self = ptr;
    return self->mode;
}

static void *stdiofile_contents(void *ptr, size_t *size)
{
    struct stdiofile *self = ptr;
    int oflag, prot;
    struct stat s;
    if (self->contents) {
        if (size) {
            *size = self->size;
        }
        return self->contents;
    }
    oflag = (self->mode&ILA_FILE_READ && self->mode&ILA_FILE_WRITE? O_RDWR :
                (self->mode&ILA_FILE_READ? O_RDONLY : 0) |
                (self->mode&ILA_FILE_WRITE? O_WRONLY : 0));
    self->fd = open(self->path, oflag);
    if (fstat(self->fd, &s) != 0) {
        return NULL;
    }
    self->size = s.st_size;
    if (size) {
        *size = self->size;
    }
    prot = (self->mode&ILA_FILE_READ?  PROT_READ  : 0) |
           (self->mode&ILA_FILE_WRITE? PROT_WRITE : 0) |
           (self->mode&ILA_FILE_EXEC?  PROT_EXEC  : 0);
    self->contents = mmap(NULL, self->size, prot, MAP_SHARED, self->fd, 0);
    return self->contents;
}


il_type il_stdiofile_type = {
    .typeclasses = NULL,
    .storage = NULL,
    .constructor = NULL,
    .destructor = stdiofile_des,
    .copy = NULL,
    .name = "il.asset.stdiofile",
    .registry = NULL,
    .size = sizeof(struct stdiofile),
    .parent = NULL
};

static ilA_file il_stdiofile_file = {
    .name = "il.asset.file",
    .mode = stdiofile_mode,
    .contents = stdiofile_contents
};

void ilA_stdiofile_init()
{
    il_impl(&il_stdiofile_type, &il_stdiofile_file);
}

char *strdup(const char*);
il_base *ilA_stdiofile(const ilA_path *path, enum ilA_file_mode mode, const ilA_file **res)
{
    struct stdiofile *f = il_new(&il_stdiofile_type);
    f->mode = mode;
    f->path = ilA_path_tochars(path);
    if (res) {
        *res = &il_stdiofile_file;
    }
    return &f->base;
}

