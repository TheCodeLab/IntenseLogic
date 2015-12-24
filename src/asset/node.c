#include "node.h"

#include <assert.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#endif

#include "util/log.h"

#ifdef _WIN32
char *strndup(const char*, size_t);
#define last_error GetLastError()
#else
#define last_error errno
#endif

#define errno_check2(e, fn, cond) \
    if (cond) { \
        (e).type = ILA_ERRNOERR; \
        (e).func = fn; \
        (e).file = __FILE__; \
        (e).line = __LINE__; \
        (e).val.err = last_error; \
        return false; \
    }
#define errno_check(err, cond) errno_check2(err, #cond, cond)
#define const_check2(e, fn, str, cond) \
    if (cond) { \
        (e).type = ILA_CONSTERR; \
        (e).func = fn; \
        (e).file = __FILE__; \
        (e).line = __LINE__; \
        (e).val.cstr = str; \
        return false; \
    }
#define const_check(err, str, cond) const_check2(err, #cond, str, cond)
#define const_error(e, fn, str) \
    (e).type = ILA_CONSTERR; \
    (e).func = fn; \
    (e).file = __FILE__; \
    (e).line = __LINE__; \
    (e).val.cstr = str; \
    return false;
#define format_error(e, fn, ...) \
    (e).type = ILA_STRERR; \
    (e).func = fn; \
    (e).file = __FILE__; \
    (e).line = __LINE__; \
    { \
    size_t err_len = snprintf(NULL, 0, __VA_ARGS__); \
    char *err_buf = malloc(err_len + 1); \
    snprintf(err_buf, err_len + 1, __VA_ARGS__); \
    (e).val.str = err_buf; \
    return false; \
    }

static bool node_test(ilA_error *err,
                      ilA_dirhandle dir,
                      const char *name, size_t namelen)
{
    (void)namelen;
#if defined(_WIN32)
#elif defined(__APPLE__)
    size_t sizeof_fullpath = strlen(dir) + 1 + namelen + 1;
    char *fullpath = malloc(sizeof_fullpath);
    snprintf(fullpath, sizeof_fullpath, "%s/%s", dir, name);
    errno_check(*err, access(fullpath, F_OK));
    free(fullpath);
#else
    errno_check(*err, faccessat(dir, name, F_OK, AT_EACCESS));
#endif
    return true;
}

#ifdef _WIN32
static char *windows_strerror(DWORD error)
{
    DWORD size = 1023;
    char* pBuffer = calloc(1, size + 1);

    size = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        pBuffer,
        size, NULL );
    pBuffer[size] = 0;

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
#ifdef _WIN32
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
    size_t size = strlen(dir) + 1 + namelen + 1;
    char *fullpath = malloc(size);
    snprintf(fullpath, size, "%s/%s", dir, name);
    *handle = CreateFileA(fullpath, access, 0, NULL, create, FILE_ATTRIBUTE_NORMAL, NULL);
    free(fullpath);
    errno_check2(*err, "CreateFileA", INVALID_HANDLE_VALUE == *handle);
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
#ifdef __APPLE__
    char fullpath[strlen(dir) + 1 + namelen + 1];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, name);
    *handle = open(fullpath, oflag);
#else
    *handle = openat(dir, name, oflag);
#endif
    errno_check2(*err, "openat", !*handle);
#endif
    return true;
}

bool ilA_fileopen(ilA_fs *fs, ilA_file *file, const char *name, size_t namelen)
{
    memset(file, 0, sizeof(ilA_file));
    if (namelen == (size_t)-1) {
        namelen = strlen(name);
    }
    for (unsigned i = 0; i < fs->dirs.length; i++) {
        if (!node_test(&file->err, fs->dirs.data[i].dir, name, (size_t)namelen)) {
            continue;
        }
        if (!ilA_dir_fileopen(fs, (ilA_dirid){i}, file, name, namelen)) {
            continue;
        }
        return true;
    }
    format_error(file->err, __func__, "No such file \"%s\"", name);
}

void ilA_fileclose(ilA_file *file)
{
#ifdef _WIN32
    CloseHandle(file->handle);
#else
    if (file->err.type == ILA_STRERR) {
        free(file->err.val.str);
    } else if (file->err.type == ILA_NOERR) {
        free(file->name);
        close(file->handle);
    }
#endif
}

bool ilA_mapfile(ilA_fs *fs, ilA_map *map, ilA_file_mode mode, const char *name, size_t namelen)
{
     memset(map, 0, sizeof(ilA_map));
     if (namelen == (size_t)-1) {
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
     format_error(map->err, __func__, "No such file \"%s\"", name);
}

bool ilA_dir_fileopen(ilA_fs *fs, ilA_dirid id, ilA_file *file, const char *name, size_t namelen)
{
    memset(file, 0, sizeof(ilA_file));
    ilA_dir *dir = &fs->dirs.data[id.id];
    if (namelen == (size_t)-1) {
        namelen = strlen(name);
    }
    if (!node_open(&file->handle, &file->err, dir->dir, name, (size_t)namelen, ILA_READ)) {
        return false;
    }
    size_t fp_len = dir->pathlen + 1 + (size_t)namelen + 1; // / and \0
    char *fullpath = malloc(fp_len);
    snprintf(fullpath, fp_len, "%s/%s", dir->path, name);
    file->name = fullpath;
    file->namelen = fp_len;
    return true;
}

bool ilA_dir_mapfile(ilA_fs *fs, ilA_dirid id, ilA_map *map, ilA_file_mode mode, const char *name, size_t namelen)
{
    memset(map, 0, sizeof(ilA_map));
    ilA_dir *dir = &fs->dirs.data[id.id];
    ilA_filehandle fh;
    if (namelen == (size_t)-1) {
        namelen = strlen(name);
    }
    if (!node_open(&fh, &map->err, dir->dir, name, (size_t)namelen, mode)) {
        return false;
    }
#ifdef _WIN32
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

    LARGE_INTEGER size;
    errno_check(map->err, GetFileSizeEx(fh, &size));
    map->size = (size_t)size.QuadPart;
    map->h.mhandle = CreateFileMapping(map->h.fhandle, NULL, prot, size.u.HighPart, size.u.LowPart, NULL);
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
    size_t fp_len = dir->pathlen + 1 + (size_t)namelen + 1; // / and \0
    char *fullpath = malloc(fp_len);
    snprintf(fullpath, fp_len, "%s/%s", dir->path, name);
    map->name = fullpath;
    map->namelen = fp_len;
    return true;
}

bool ilA_mapopen(ilA_map *map, ilA_file_mode mode, ilA_file file)
{
    memset(map, 0, sizeof(ilA_map));
#ifdef _WIN32
    assert(mode < 8);
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
    DWORD prot = prot_table[mode & ILA_RWE];
    DWORD oflag = oflag_table[mode & ILA_RWE];
    DWORD access = flag_table[mode & ILA_RWE];
    BOOL res = DuplicateHandle
        (GetCurrentProcess(),
         file.handle,
         GetCurrentProcess(),
         &map->h.fhandle,
         0,
         FALSE, // can be inherited
         DUPLICATE_SAME_ACCESS); // options
    errno_check2(map->err, "DuplicateHandle", 0 == res);
    LARGE_INTEGER size;
    errno_check(map->err, 0 == GetFileSizeEx(map->h.fhandle, &size));
    map->size = (size_t)size.QuadPart;
    map->h.mhandle = CreateFileMapping(map->h.fhandle, NULL, prot, size.u.HighPart, size.u.LowPart, NULL);
    errno_check2(map->err, "CreateFileMapping", NULL == map->h.mhandle);
    map->data = MapViewOfFile(map->h.mhandle, oflag, 0, 0, 0);
    errno_check2(map->err, "MapViewOfFile", NULL == map->data);
#else
    map->h = dup(file.handle);
    errno_check2(map->err, "dup", map->h == -1);
    struct stat s;
    if (fstat(map->h, &s) != 0) {
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
    map->data = mmap(NULL, map->size, prot, MAP_SHARED, map->h, 0);
    errno_check2(map->err, "mmap", map->data == MAP_FAILED);
#endif
    map->name = strndup(file.name, file.namelen);
    map->namelen = file.namelen;
    map->mode = mode;
    return true;
}

void ilA_unmapfile(ilA_map *map)
{
#ifdef _WIN32
    UnmapViewOfFile(map->data);
    CloseHandle(map->h.mhandle);
    CloseHandle(map->h.fhandle);
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

ilA_filehandle ilA_rawopen(ilA_fs *fs, ilA_error *err, ilA_file_mode mode, const char *name, size_t namelen)
{
    if (namelen == (size_t)-1) {
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
     format_error(*err, __func__, "No such file \"%s\"", name);
     return ilA_invalid_file;
}

ilA_filehandle ilA_dir_rawopen(ilA_fs *fs, ilA_dirid id, ilA_error *err, ilA_file_mode mode, const char *name, size_t namelen)
{
    ilA_filehandle h;
    if (!node_open(&h, err, fs->dirs.data[id.id].dir, name, (size_t) namelen, mode)) {
        return ilA_invalid_file;
    }
    return h;
}

ilA_dirid ilA_adddir(ilA_fs *fs, const char *path, size_t pathlen)
{
    ilA_dir d;
    d.path = strdup(path);
    if (pathlen == (size_t)-1) {
        pathlen = strlen(path);
    }
    d.pathlen = (size_t)pathlen;
#if defined(_WIN32) || defined(__APPLE__)
    d.dir = strdup(path);
#else
    d.dir = open(path, O_DIRECTORY);
#endif
    IL_APPEND(fs->dirs, d);
    return (ilA_dirid){fs->dirs.length - 1};
}

void ilA_deldir(ilA_fs *fs, ilA_dirid id)
{
#if defined(_WIN32) || defined(__APPLE__)
    free(fs->dirs.data[id.id].dir);
#else
    close(fs->dirs.data[id.id].dir);
#endif
    IL_REMOVE(fs->dirs, id.id);
}

ilA_dirid ilA_mkdir(ilA_fs *fs, const char *path, size_t pathlen)
{
    (void)fs, (void)path, (void)pathlen;
    assert(!"ilA_mkdir unimplemented");
    return (ilA_dirid){0};
}

void ilA_rmdir(ilA_fs *fs, ilA_dirid id)
{
    (void)fs, (void)id;
    assert(!"ilA_rmdir unimplemented");
}

void ilA_delete(ilA_fs *fs, const char *name, size_t namelen)
{
    (void)fs, (void)name, (void)namelen;
    assert(!"ilA_delete unimplemented");
}

void ilA_lookup(ilA_fs *fs, const char *pattern, size_t patlen, void (*cb)(void *user, ilA_dirid id, const char *name, size_t namelen), void *user)
{
    (void)fs, (void)pattern, (void)patlen, (void)cb, (void)user;
    assert(!"ilA_lookup unimplemented");
}

int ilA_strerror(ilA_error *err, char *buf, size_t len)
{
    switch (err->type) {
    case ILA_NOERR:
        return snprintf(buf, len, "No error: Something reported an error when there is none");
        break;
    case ILA_ERRNOERR: {
#ifdef _WIN32
        char *error = windows_strerror(err->val.err);
        int res = snprintf(buf, len, "[%s:%i] %s: %s", err->file, err->line, err->func, error);
        free(error);
        return res;
#else
        return snprintf(buf, len, "[%s:%i] %s: %s", err->file, err->line, err->func, strerror(err->val.err));
#endif
        break;
    }
    case ILA_STRERR:
    case ILA_CONSTERR:
        return snprintf(buf, len, "[%s:%i] %s: %s", err->file, err->line, err->func, err->val.cstr);
        break;
    default:
        return snprintf(buf, len, "Corrupt error value");
    }
}

char *ilA_strerrora(ilA_error *err, size_t *len)
{
    char *buf = malloc(1024); // cheesy solution
    int res = ilA_strerror(err, buf, 1024);
    if (res < 0) {
        return strdup("sprintf failure");
    }
    size_t size = (size_t)res;
    if (len) {
        *len = size;
    }
    return realloc(buf, size);
}

void ilA_printerror_real(ilA_error *err, const char *file, int line, const char *func)
{
#define err(...) il_log_real(file, line, func, 0, __VA_ARGS__)
    switch (err->type) {
    case ILA_NOERR:
        err("No error: Reported an error when there is none");
        break;
    case ILA_ERRNOERR:
#ifdef _WIN32
        err("From %s:%i (%s): %s", err->file, err->line, err->func, windows_strerror(err->val.err));
#else
        err("From %s:%i (%s): %s", err->file, err->line, err->func, strerror(err->val.err));
#endif
        break;
    case ILA_STRERR:
    case ILA_CONSTERR:
        err("From %s:%i (%s): %s", err->file, err->line, err->func, err->val.cstr);
        break;
    default:
        err("Corrupt error value");
    }
}
