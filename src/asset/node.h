/** @file node.h
 * @brief VFS code
 *
 * All the operations have their parameters setup as follows:
 *
 *      {iface} {obj} [path] [out size, out data] ... [result iface]
 */

#ifndef ILA_NODE_H
#define ILA_NODE_H

#include <stdlib.h>

#include "asset/path.h"
#include "common/base.h"

enum ilA_file_mode {
    ILA_FILE_READ   = 1<<0,
    ILA_FILE_WRITE  = 1<<1,
    ILA_FILE_EXEC   = 1<<2
};

typedef struct ilA_file ilA_file;
typedef struct ilA_dir ilA_dir;

typedef enum ilA_file_mode (*ilA_file_mode_fn)(void *self);
typedef void *(*ilA_file_contents_fn)(void *self, size_t *size);

/** Typeclass for file operations */
struct ilA_file {
    il_typeclass_header;
    ilA_file_mode_fn mode;
    ilA_file_contents_fn contents;
};

typedef il_base *(*ilA_dir_lookup_fn)(void *self, const ilA_path *path);
typedef il_base *(*ilA_dir_create_fn)(void *self, const ilA_path *path, const ilA_file **res);
typedef il_base *(*ilA_dir_mkdir_fn) (void *self, const ilA_path *path, const ilA_dir **res);
typedef void     (*ilA_dir_delete_fn)(void *self, const ilA_path *path);

/** Typeclass for directory operations */
struct ilA_dir {
    il_typeclass_header;
    ilA_dir_lookup_fn lookup;
    ilA_dir_create_fn create;
    ilA_dir_mkdir_fn mkdir;
    ilA_dir_delete_fn del;
};

/** Creates a file object from an object that resides on the real file system */
il_base *ilA_stdiofile      (const ilA_path *path,  enum ilA_file_mode mode,                    const ilA_file **res);
/** Creates a directory object from a directory that resides on the real file system */
il_base *ilA_stdiodir       (const ilA_path *path,  const ilA_dir **res);
/** Creates a directory union from two directories
 *
 * If one of the two directories is a union directory itself, it will extend that to a 3-directory union. */
il_base *ilA_union          (const ilA_dir  *ai,    const ilA_dir *bi, il_base *a, il_base *b,  const ilA_dir **res);
/** Wraps a directory such that its real contents are behind a path 
 *
 * A directory with foo, bar, and baz inside of it and a prefix of /a/b/c would become /a/b/c/foo, /a/b/c/bar, and /a/b/c/baz.*/
il_base *ilA_prefix         (const ilA_dir  *iface, il_base *dir, const ilA_path *path,         const ilA_dir **res);
/** Looks up a node in a directory */
il_base *ilA_lookup         (const ilA_dir  *iface, il_base *dir, const ilA_path *path);
/** Creates a new file in a directory */
il_base *ilA_create         (const ilA_dir  *iface, il_base *dir, const ilA_path *path,         const ilA_file **res);
/** Creates a new directory in a directory */
il_base *ilA_mkdir          (const ilA_dir  *iface, il_base *dir, const ilA_path *path,         const ilA_dir **res);
/** Deletes a node in a directory */
void     ilA_delete         (const ilA_dir  *iface, il_base *dir, const ilA_path *path);
/** Returns the contents of a file */
void    *ilA_contents       (const ilA_file *iface, il_base *file, size_t *size);
/** Opens a real file and returns its contents */
il_base *ilA_contents_path  (const ilA_path *path, size_t *size, void **data, const ilA_file **res);
/** Opens a real file and returns its contents */
il_base *ilA_contents_chars (const char *path, size_t *size, void **data, const ilA_file **res);

#endif

