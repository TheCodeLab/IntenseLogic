/** @file node.h
 * @brief VFS code
 */

#ifndef ILA_NODE_H
#define ILA_NODE_H

#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "util/array.h"

#ifdef WIN32
typedef HANDLE ilA_filehandle;
typedef struct ilA_maphandle {
    HANDLE fhandle, mhandle;
} ilA_maphandle;
typedef DWORD ilA_errno_t;
#else
typedef int ilA_filehandle;
typedef int ilA_maphandle;
typedef int ilA_dirhandle;
typedef int ilA_errno_t;
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
    const char *func;
    union {
        ilA_errno_t err;
        char *str;
        const char *cstr;
    } val;
} ilA_error;

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

bool ilA_mapfile(ilA_fs *fs, ilA_map *map, ilA_file_mode mode, const char *name, ssize_t namelen);
bool ilA_dir_mapfile(ilA_fs *fs, ilA_dirid id, ilA_map *map, ilA_file_mode mode, const char *name, ssize_t namelen);
void ilA_unmapfile(ilA_map *map);
int ilA_rawopen(ilA_fs *fs, ilA_error *err, ilA_file_mode mode, const char *name, ssize_t namelen);
int ilA_dir_rawopen(ilA_fs *fs, ilA_dirid id, ilA_error *err, ilA_file_mode mode, const char *name, ssize_t namelen);
ilA_dirid ilA_adddir(ilA_fs *fs, const char *path, ssize_t pathlen);
void ilA_deldir(ilA_fs *fs, ilA_dirid id);
ilA_dirid ilA_mkdir(ilA_fs *fs, const char *path, ssize_t pathlen);
void ilA_rmdir(ilA_fs *fs, ilA_dirid id);
void ilA_delete(ilA_fs *fs, const char *name, ssize_t namelen);
void ilA_lookup(ilA_fs *fs, const char *pattern, ssize_t patlen, void (*cb)(void *user, ilA_dirid id, const char *name, size_t namelen), void *user);
void ilA_strerror(ilA_error *err, char *buf, size_t len);
void ilA_printerror(ilA_error *err);

#endif
