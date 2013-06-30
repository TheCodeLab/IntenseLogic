#include "node.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#ifdef WIN32
# include <windows.h>
#else
# include <sys/mman.h>
# include <fcntl.h>
#endif

struct stdiofile {
    il_base base;
    void *contents;
    size_t size;
    char *path;
    enum ilA_file_mode mode;
#ifdef WIN32
    HANDLE handle;
#else
    int fd;
#endif
};

static void stdiofile_des(void *ptr)
{
    struct stdiofile *self = ptr;
#ifdef WIN32
    UnmapViewOfFile(self->contents);
    CloseHandle(self->handle);
#else
    munmap(self->contents, self->size);
    close(self->fd);
#endif
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
    int oflag;
#ifndef WIN32
    int prot;
    struct stat s;
#endif

    if (self->contents) {
        if (size) {
            *size = self->size;
        }
        return self->contents;
    }
#ifdef WIN32
    oflag = (self->mode&ILA_FILE_READ?   FILE_MAP_READ    : 0) |
            (self->mode&ILA_FILE_WRITE?  FILE_MAP_WRITE   : 0);// |
            //(self->mode&ILA_FILE_EXEC?   FILE_MAP_EXECUTE : 0); // TODO: find out why mingw thinks there's no such thing as executable files
    self->handle = OpenFileMappingA(oflag, FALSE, self->path);
    self->size = GetFileSize(self->handle, NULL);
    self->contents = MapViewOfFile(self->handle, oflag, 0, 0, self->size);
#else
    oflag = (self->mode&ILA_FILE_READ && self->mode&ILA_FILE_WRITE? O_RDWR :
                (self->mode&ILA_FILE_READ? O_RDONLY : 0) |
                (self->mode&ILA_FILE_WRITE? O_WRONLY : 0));
    self->fd = open(self->path, oflag);
    if (fstat(self->fd, &s) != 0) {
        return NULL;
    }
    self->size = s.st_size;
    prot = (self->mode&ILA_FILE_READ?  PROT_READ  : 0) |
           (self->mode&ILA_FILE_WRITE? PROT_WRITE : 0) |
           (self->mode&ILA_FILE_EXEC?  PROT_EXEC  : 0);
    self->contents = mmap(NULL, self->size, prot, MAP_SHARED, self->fd, 0);
#endif
    if (size) {
        *size = self->size;
    }
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
    if (access(f->path, F_OK)) {
        il_unref(f);
        return NULL;
    }
    if (res) {
        *res = &il_stdiofile_file;
    }
    return &f->base;
}

