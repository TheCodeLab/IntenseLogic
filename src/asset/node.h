/** @file node.h
 * @brief VFS code
 */

#ifndef ILA_NODE_H
#define ILA_NODE_H

#include <stdlib.h>
#include <stdbool.h>

#include "util/array.h"
#include "util/types.h"

#ifdef _WIN32
#include <windows.h>
typedef HANDLE ilA_filehandle;
typedef struct ilA_maphandle {
    HANDLE fhandle, mhandle;
} ilA_maphandle;
typedef char *ilA_dirhandle;
typedef DWORD ilA_errno_t;
#define ilA_invalid_file NULL
#elif __APPLE__
#include <unistd.h>
typedef int ilA_filehandle;
typedef int ilA_maphandle;
typedef char *ilA_dirhandle;
typedef int ilA_errno_t;
#define ilA_invalid_file -1
#else
#include <unistd.h>
typedef int ilA_filehandle;
typedef int ilA_maphandle;
typedef int ilA_dirhandle;
typedef int ilA_errno_t;
#define ilA_invalid_file -1
#endif

typedef enum ilA_file_mode {
    ILA_READ   = 1<<0,
    ILA_WRITE  = 1<<1,
    ILA_EXEC   = 1<<2,
    ILA_CREATE = 1<<3,
    ILA_RW     = ILA_READ|ILA_WRITE,
    ILA_RWE    = ILA_RW | ILA_EXEC,
    ILA_RWC    = ILA_RW | ILA_CREATE
} ilA_file_mode;

typedef struct ilA_error {
    enum {
        ILA_NOERR,
        ILA_ERRNOERR,
        ILA_STRERR,
        ILA_CONSTERR
    } type;
    const char *func, *file;
    int line;
    union {
        ilA_errno_t err;
        char *str;
        const char *cstr;
    } val;
} ilA_error;

typedef struct ilA_file {
    char *name;
    size_t namelen;
    ilA_filehandle handle;
    ilA_error err;
} ilA_file;

typedef struct ilA_map {
    char *name;
    size_t namelen;
    void *data;
    size_t size;
    ilA_file_mode mode;
    ilA_maphandle h;
    ilA_error err;
} ilA_map;

typedef struct ilA_dir {
    char *path;
    size_t pathlen;
    ilA_dirhandle dir;
} ilA_dir;

typedef struct ilA_dirid {
    unsigned id;
} ilA_dirid;

typedef struct ilA_fs {
    ilA_dirid writedir;
    IL_ARRAY(ilA_dir,) dirs;
} ilA_fs;

IL_WARN_UNUSED
bool ilA_fileopen(ilA_fs *fs, ilA_file *file, const char *name, size_t namelen);
void ilA_fileclose(ilA_file *file);
IL_WARN_UNUSED
bool ilA_mapfile(ilA_fs *fs, ilA_map *map, ilA_file_mode mode, const char *name, size_t namelen);
IL_WARN_UNUSED
bool ilA_dir_fileopen(ilA_fs *fs, ilA_dirid id, ilA_file *file, const char *name, size_t namelen);
IL_WARN_UNUSED
bool ilA_dir_mapfile(ilA_fs *fs, ilA_dirid id, ilA_map *map, ilA_file_mode mode, const char *name, size_t namelen);
IL_WARN_UNUSED
bool ilA_mapopen(ilA_map *map, ilA_file_mode mode, ilA_file file);
void ilA_unmapfile(ilA_map *map);
ilA_filehandle ilA_rawopen(ilA_fs *fs, ilA_error *err, ilA_file_mode mode, const char *name, size_t namelen);
ilA_filehandle ilA_dir_rawopen(ilA_fs *fs, ilA_dirid id, ilA_error *err, ilA_file_mode mode, const char *name, size_t namelen);
ilA_dirid ilA_adddir(ilA_fs *fs, const char *path, size_t pathlen);
void ilA_deldir(ilA_fs *fs, ilA_dirid id);
ilA_dirid ilA_mkdir(ilA_fs *fs, const char *path, size_t pathlen);
void ilA_rmdir(ilA_fs *fs, ilA_dirid id);
void ilA_delete(ilA_fs *fs, const char *name, size_t namelen);
void ilA_lookup(ilA_fs *fs, const char *pattern, size_t patlen, void (*cb)(void *user, ilA_dirid id, const char *name, size_t namelen), void *user);
int ilA_strerror(ilA_error *err, char *buf, size_t len);
char *ilA_strerrora(ilA_error *err, size_t *len);
void ilA_printerror_real(ilA_error *err, const char *file, int line, const char *func);
#define ilA_printerror(e) ilA_printerror_real(e, __FILE__, __LINE__, __func__)

#endif
