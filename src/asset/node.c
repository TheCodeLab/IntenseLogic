#include "node.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <assert.h>

#include "util/log.h"
#include "util/ilassert.h"

#ifdef WIN32
#define last_error GetLastError()
#else
#define last_error errno
#endif

#define errno_check2(e, fn, cond) \
    if (cond) { \
        (e).type = ILA_ERRNOERR; \
        (e).func = fn; \
        (e).val.err = last_error; \
        return false; \
    }
#define errno_check(err, cond) errno_check2(err, #cond, cond)
#define const_check2(e, fn, str, cond) \
    if (cond) { \
        (e).type = ILA_CONSTERR; \
        (e).func = fn; \
        (e).val.cstr = str; \
        return false; \
    }
#define const_check(err, str, cond) const_check2(err, #cond, str, cond)

static bool node_test(ilA_error *err,
                      ilA_dirhandle dir,
                      const char *name, size_t namelen)
{
    (void)namelen;
#ifdef WIN32

#else
    errno_check(*err, faccessat(dir, name, F_OK, AT_EACCESS));
#endif
    return true;
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

static bool node_open(ilA_filehandle *handle,
                      ilA_error *err,
                      ilA_dirhandle dir,
                      const char *name, size_t namelen,
                      ilA_file_mode mode)
{
    (void)namelen;
#ifdef WIN32
    static const int flag_table[] = {
        -1,
        GENERIC_READ,
        GENERIC_WRITE,
        GENERIC_READ|GENERIC_WRITE,
        GENERIC_EXECUTE,
        GENERIC_READ|GENERIC_EXECUTE,
        GENERIC_WRITE|GENERIC_EXECUTE,
        GENERIC_READ|GENERIC_WRITE|GENERIC_EXECUTE
    };

    const_check(*err, "Invalid mode flag", mode == 0);
    DWORD access = flag_table[mode & ILA_RWE];
    DWORD create = (mode & ILA_CREATE)? CREATE_NEW : OPEN_EXISTING;
    *handle = CreateFileA(self->path, access, 0, NULL, create, FILE_ATTRIBUTE_NORMAL, NULL);
    errno_check2(*err, "CreateFileA", !*handle);
#else
    static const int flag_table[] = {  // 0bEWR
        -1,                 // 0b000 no flags
        O_RDONLY,           // 0b001
        O_WRONLY,           // 0b010
        O_RDWR,             // 0b011
        -1,                 // 0b100 EXEC, O_EXEC is missing from non-GNU
        -1,                 // 0b101 EXEC READ
        -1,                 // 0b110 EXEC WRITE
        -1,                 // 0b111 EXEC RW
    };
    int oflag = flag_table[mode & ILA_RWE];
    const_check(*err, "Invalid mode flag", oflag < 0);
    if (mode & ILA_CREATE) {
        oflag |= O_CREAT;
    }
    *handle = openat(dir, name, oflag);
    errno_check2(*err, "openat", !*handle);
#endif
    return true;
}

bool ilA_mapfile(ilA_fs *fs, ilA_map *map, ilA_file_mode mode, const char *name, ssize_t namelen)
{
     memset(map, 0, sizeof(ilA_map));
     if (namelen < 0) {
         namelen = strlen(name);
     }
     for (unsigned i = 0; i < fs->dirs.length; i++) {
         if (!node_test(&map->err, fs->dirs.data[i].dir, name, (size_t)namelen)) {
             continue;
         }
         if (!ilA_dir_mapfile(fs, (ilA_dirid){i}, map, mode, name, namelen)) {
             continue;
         }
         return true;
     }
     const_check(map->err, "No such file", true);
}

bool ilA_dir_mapfile(ilA_fs *fs, ilA_dirid id, ilA_map *map, ilA_file_mode mode, const char *name, ssize_t namelen)
{
    memset(map, 0, sizeof(ilA_map));
    ilA_dir *dir = &fs->dirs.data[id.id];
    ilA_filehandle fh;
    if (namelen < 0) {
        namelen = strlen(name);
    }
    if (!node_open(&fh, &map->err, dir->dir, name, (size_t)namelen, mode)) {
        return false;
    }
#ifdef WIN32
    static const int prot_table[] = {
        -1,                     // 0b000
        PAGE_READONLY,          // 0b001
        PAGE_WRITECOPY,         // 0b010
        PAGE_READWRITE,         // 0b011
        -1,                     // 0b100
        PAGE_EXECUTE_READ,      // 0b101
        PAGE_EXECUTE_WRITECOPY, // 0b110
        PAGE_EXECUTE_READWRITE  // 0b111
    };
    static const int oflag_table[] = {
        -1,
        FILE_MAP_READ,
        FILE_MAP_WRITE,
        FILE_MAP_READ|FILE_MAP_WRITE,
        FILE_MAP_EXECUTE,
        FILE_MAP_READ|FILE_MAP_EXECUTE,
        FILE_MAP_WRITE|FILE_MAP_EXECUTE,
        FILE_MAP_READ|FILE_MAP_WRITE|FILE_MAP_EXECUTE
    };
    DWORD prot = prot_table[mode & ILA_RWE];
    DWORD oflag = oflag_table[mode & ILA_RWE];
    errno_check(map->err, GetFileSizeEx(fh, &map->size));
    map->h.mhandle = CreateFileMapping(self->fhandle, NULL, prot, high, low, NULL);
    errno_check2(map->err, "CreateFileMapping", !map->h.mhandle);
    map->data = MapViewOfFile(map->h.mhandle, oflag, 0, 0, 0);
    errno_check2(map->err, "MapViewOfFile", !map->data);
    map->h.fhandle = fh;
#else
    struct stat s;
    if (fstat(fh, &s) != 0) {
        map->err.type = ILA_ERRNOERR;
        map->err.func = "fstat";
        map->err.val.err = errno;
        return false;
    }
    map->size = s.st_size;
    static const int prot_table[] = {   // 0bRWE
        PROT_NONE,                      // 0b000
        PROT_READ,                      // 0b001
        PROT_WRITE,                     // 0b010
        PROT_READ|PROT_WRITE,           // 0b011
        PROT_EXEC,                      // 0b100
        PROT_READ|PROT_EXEC,            // 0b101 EXEC READ
        PROT_WRITE|PROT_EXEC,           // 0b110 EXEC WRITE
        PROT_READ|PROT_WRITE|PROT_EXEC, // 0b111 EXEC READ WRITE
    };
    int prot = prot_table[mode & ILA_RWE];
    map->data = mmap(NULL, map->size, prot, MAP_SHARED, fh, 0);
    errno_check2(map->err, "mmap", map->data == MAP_FAILED);
    map->h = fh;
#endif
    map->name = strndup(name, (size_t)namelen);
    map->namelen = (size_t)namelen;
    map->mode = mode;
    return true;
}

void ilA_unmapfile(ilA_map *map)
{
#ifdef WIN32

#else
    if (map->err.type == ILA_STRERR) {
        free(map->err.val.str);
    } else if (map->err.type == ILA_NOERR) {
        free(map->name);
        munmap(map->data, map->size);
        close(map->h);
    }
#endif
}

int ilA_rawopen(ilA_fs *fs, ilA_error *err, ilA_file_mode mode, const char *name, ssize_t namelen)
{
     if (namelen < 0) {
         namelen = strlen(name);
     }
     for (unsigned i = 0; i < fs->dirs.length; i++) {
         if (!node_test(err, fs->dirs.data[i].dir, name, (size_t)namelen)) {
             continue;
         }
         ilA_filehandle h;
         if (!node_open(&h, err, fs->dirs.data[i].dir, name, (size_t)namelen, mode)) {
             continue;
         }
         return h;
     }
     err->type = ILA_CONSTERR;
     err->val.cstr = "No such file";
     return -1;
}

int ilA_dir_rawopen(ilA_fs *fs, ilA_dirid id, ilA_error *err, ilA_file_mode mode, const char *name, ssize_t namelen)
{
    ilA_filehandle h;
    if (!node_open(&h, err, fs->dirs.data[id.id].dir, name, (size_t) namelen, mode)) {
        return -1;
    }
    return h;
}

ilA_dirid ilA_adddir(ilA_fs *fs, const char *path, ssize_t pathlen)
{
    ilA_dir d;
    d.path = strdup(path);
    if (pathlen < 0) {
        pathlen = strlen(path);
    }
    d.pathlen = (size_t)pathlen;
#ifdef WIN32
#else
    d.dir = open(path, O_DIRECTORY);
#endif
    IL_APPEND(fs->dirs, d);
    return (ilA_dirid){fs->dirs.length - 1};
}

void ilA_deldir(ilA_fs *fs, ilA_dirid id)
{
#ifdef WIN32
#else
    close(fs->dirs.data[id.id].dir);
#endif
    IL_REMOVE(fs->dirs, id.id);
}

ilA_dirid ilA_mkdir(ilA_fs *fs, const char *path, ssize_t pathlen)
{
    (void)fs, (void)path, (void)pathlen;
    assert(!"ilA_mkdir unimplemented");
}

void ilA_rmdir(ilA_fs *fs, ilA_dirid id)
{
    (void)fs, (void)id;
    assert(!"ilA_rmdir unimplemented");
}

void ilA_delete(ilA_fs *fs, const char *name, ssize_t namelen)
{
    (void)fs, (void)name, (void)namelen;
    assert(!"ilA_delete unimplemented");
}

void ilA_lookup(ilA_fs *fs, const char *pattern, ssize_t patlen, void (*cb)(void *user, ilA_dirid id, const char *name, size_t namelen), void *user)
{
    (void)fs, (void)pattern, (void)patlen, (void)cb, (void)user;
    assert(!"ilA_lookup unimplemented");
}

void ilA_strerror(ilA_error *err, char *buf, size_t len)
{
    switch (err->type) {
    case ILA_NOERR:
        snprintf(buf, len, "No error: Something reported an error when there is none");
        break;
    case ILA_ERRNOERR:
#ifdef WIN32
        snprintf(buf, len, "%s: %s", err->func, windows_strerror(err->val.err));
#else
        snprintf(buf, len, "%s: %s", err->func, strerror(err->val.err));
#endif
        break;
    case ILA_STRERR:
    case ILA_CONSTERR:
        snprintf(buf, len, "%s: %s", err->func, err->val.cstr);
        break;
    default:
        snprintf(buf, len, "Corrupt error value");
    }
}

void ilA_printerror(ilA_error *err)
{
#define err(...) il_log_real("Asset", 0, err->func, 0, __VA_ARGS__)
    switch (err->type) {
    case ILA_NOERR:
        err("No error: Something reported an error when there is none");
        break;
    case ILA_ERRNOERR:
#ifdef WIN32
        err("%s", windows_strerror(err->val.err));
#else
        err("%s", strerror(err->val.err));
#endif
        break;
    case ILA_STRERR:
    case ILA_CONSTERR:
        err("%s", err->val.cstr);
        break;
    default:
        err("Corrupt error value");
    }
}
