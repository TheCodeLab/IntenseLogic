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

struct ilA_file {
    il_typeclass_header;
    ilA_file_mode_fn mode;
    ilA_file_contents_fn contents;
};

typedef il_base *(*ilA_dir_lookup_fn)(void *self, const ilA_path *path);
typedef il_base *(*ilA_dir_create_fn)(void *self, const ilA_path *path, const ilA_file **res);
typedef il_base *(*ilA_dir_mkdir_fn) (void *self, const ilA_path *path, const ilA_dir **res);
typedef void     (*ilA_dir_delete_fn)(void *self, const ilA_path *path);

struct ilA_dir {
    il_typeclass_header;
    ilA_dir_lookup_fn lookup;
    ilA_dir_create_fn create;
    ilA_dir_mkdir_fn mkdir;
    ilA_dir_delete_fn delete;
};

// [iface] [obj] ... [result_iface]
// constructor functions
il_base *ilA_stdiofile      (const ilA_path *path,  enum ilA_file_mode mode,                    const ilA_file **res);
il_base *ilA_stdiodir       (const ilA_path *path,  const ilA_dir **res);
il_base *ilA_union          (const ilA_dir  *ai,    const ilA_dir *bi, il_base *a, il_base *b,  const ilA_dir **res);
il_base *ilA_prefix         (const ilA_dir  *iface, il_base *dir, const ilA_path *path,         const ilA_dir **res);
// operations
il_base *ilA_lookup         (const ilA_dir  *iface, il_base *dir, const ilA_path *path);
il_base *ilA_create         (const ilA_dir  *iface, il_base *dir, const ilA_path *path,         const ilA_file **res);
il_base *ilA_mkdir          (const ilA_dir  *iface, il_base *dir, const ilA_path *path,         const ilA_dir **res);
void     ilA_delete         (const ilA_dir  *iface, il_base *dir, const ilA_path *path);
void    *ilA_contents       (const ilA_file *iface, il_base *file, size_t *size);
il_base *ilA_contents_path  (const ilA_path *path, size_t *size, void **data, const ilA_file **res);
il_base *ilA_contents_chars (const char *path, size_t *size, void **data, const ilA_file **res);

#endif

