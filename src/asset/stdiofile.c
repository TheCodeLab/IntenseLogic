#include "node.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#ifdef WIN32
# include <windows.h>
#else
# include <sys/mman.h>
# include <fcntl.h>
#endif

#include "util/log.h"

struct stdiofile {
    il_base base;
    void *contents;
    size_t size;
    char *path;
    enum ilA_file_mode mode;
#ifdef WIN32
    HANDLE fhandle;
    HANDLE mhandle;
#else
    int fd;
#endif
};

static void stdiofile_des(void *ptr)
{
    struct stdiofile *self = ptr;
#ifdef WIN32
    UnmapViewOfFile(self->contents);
    CloseHandle(self->mhandle);
    CloseHandle(self->fhandle);
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

#ifdef WIN32
static char *windows_strerror()
{
    char* pBuffer = NULL;

    DWORD dw = GetLastError(); 

    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&pBuffer,
        0, NULL );

    return pBuffer;
}
#endif

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
    DWORD access = (self->mode&ILA_FILE_READ?  GENERIC_READ    : 0) |
                   (self->mode&ILA_FILE_WRITE? GENERIC_WRITE   : 0) |
                   (self->mode&ILA_FILE_EXEC?  GENERIC_EXECUTE : 0);
    self->fhandle = CreateFileA(self->path, access, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (!self->fhandle) {
        il_error("CreateFileA: %s", windows_strerror());
        return NULL;
    }
    DWORD prot;
    if (self->mode & ILA_FILE_READ) {
        if (self->mode & ILA_FILE_WRITE) {
            prot    = self->mode&ILA_FILE_EXEC? PAGE_EXECUTE_READWRITE : PAGE_READWRITE;
        } else {
            prot    = self->mode&ILA_FILE_EXEC? PAGE_EXECUTE_READ      : PAGE_READONLY;
        }
    } else if (self->mode & ILA_FILE_WRITE) {
        prot        = self->mode&ILA_FILE_EXEC? PAGE_EXECUTE_WRITECOPY : PAGE_WRITECOPY;
    } else {
        // win32 has no PAGE_EXECUTEONLY or null option, so each of these has read
        prot        = self->mode&ILA_FILE_EXEC? PAGE_EXECUTE_READ      : PAGE_READONLY; 
    }
    DWORD low, high;
    low = GetFileSize(self->fhandle, &high);
    self->size = low + ((size_t)high<<32);
    self->mhandle = CreateFileMapping(self->fhandle, NULL, prot, high, low, NULL);
    if (!self->mhandle) {
        il_error("CreateFileMappingA: %s", windows_strerror());
        return NULL;
    }
    oflag = (self->mode&ILA_FILE_READ?   FILE_MAP_READ    : 0) |
            (self->mode&ILA_FILE_WRITE?  FILE_MAP_WRITE   : 0) |
            (self->mode&ILA_FILE_EXEC?   FILE_MAP_EXECUTE : 0);
    self->contents = MapViewOfFile(self->mhandle, oflag, 0, 0, 0);
    if (!self->contents) {
        il_error("MapViewOfFileA: %s", windows_strerror());
        return NULL;
    }
#else
    oflag = (self->mode&ILA_FILE_READ && self->mode&ILA_FILE_WRITE? O_RDWR :
                (self->mode&ILA_FILE_READ? O_RDONLY : 0) |
                (self->mode&ILA_FILE_WRITE? O_WRONLY : 0));
    self->fd = open(self->path, oflag);
    if (fstat(self->fd, &s) != 0) {
        il_error("fstat: %s", strerror(errno));
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

