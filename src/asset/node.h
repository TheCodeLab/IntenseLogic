#ifndef ILA_NODE_H
#define ILA_NODE_H

#include <stdlib.h>

#include "asset/path.h"

enum ilA_node_type {
    ILA_NODE_NODE,
    ILA_NODE_FILE,
    ILA_NODE_DIR
};

typedef struct ilA_node {
    enum ilA_node_type type;
    const char *impl;
    int refs;
    void (*free)(struct ilA_node* self);
    void *user;
} ilA_node;

enum ilA_file_mode {
    ILA_FILE_READ   = 1<<0,
    ILA_FILE_WRITE  = 1<<1,
    ILA_FILE_EXEC   = 1<<2
};

typedef struct ilA_file {
    ilA_node node;
    enum ilA_file_mode mode;
    void *(*contents)(struct ilA_file* self, size_t *size);
} ilA_file;

typedef struct ilA_dir {
    ilA_node node;
    ilA_node        *(*lookup)(struct ilA_dir* self, const ilA_path *path);
    ilA_file        *(*create)(struct ilA_dir* self, const ilA_path *path);
    struct ilA_dir  *(*mkdir) (struct ilA_dir* self, const ilA_path *path);
    void             (*delete)(struct ilA_dir* self, const ilA_path *path);
} ilA_dir;

#define ILA_FILE(node) ((ilA_file*)(node->type == ILA_NODE_FILE? node : NULL))
#define ILA_DIR(node) ((ilA_dir*)(node->type == ILA_NODE_DIR? node : NULL))

ilA_node *ilA_node_ref(void* node);
void ilA_node_unref(void* node);
ilA_file *ilA_node_stdio_file(const ilA_path *path, enum ilA_file_mode mode);
ilA_dir *ilA_node_stdio_dir(const ilA_path *path);
ilA_dir *ilA_node_union(ilA_dir *a, ilA_dir *b);
ilA_dir *ilA_node_prefix(ilA_dir *dir, const ilA_path *path);
ilA_node *ilA_node_lookup(ilA_dir* dir, const ilA_path *path);
void *ilA_node_contents(ilA_file* file, size_t *size);

#endif

