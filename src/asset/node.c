#include "node.h"

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>

//#include <mowgli.h>

ilA_node *ilA_node_ref(void* ptr) 
{
    ilA_node* node = ptr;
    node->refs++;
    return node;
}

void ilA_node_unref(void* ptr)
{
    ilA_node* node = ptr;
    node->refs--;
    if (node->refs == 0) {
        node->free(node);
    }
}

struct stdio_ctx {
    int fd;
    void *contents;
    size_t size;
    char *path;
};

static void stdio_free(ilA_node *self)
{
    struct stdio_ctx *ctx = self->user;
    munmap(ctx->contents, ctx->size);
    close(ctx->fd);
    free(ctx->path);
    free(ctx);
    free(self);
}

static void *stdio_contents(ilA_file* self, size_t *size)
{
    struct stdio_ctx *ctx = self->node.user;
    if (ctx->contents) {
        if (size) {
            *size = ctx->size;
        }
        return ctx->contents;
    }
    int oflag = (self->mode&ILA_FILE_READ && self->mode&ILA_FILE_WRITE? O_RDWR :
                    (self->mode&ILA_FILE_READ? O_RDONLY : 0) |
                    (self->mode&ILA_FILE_WRITE? O_WRONLY : 0));
    ctx->fd = open(ctx->path, oflag);
    struct stat s;
    if (fstat(ctx->fd, &s) != 0) {
        return NULL;
    }
    ctx->size = s.st_size;
    if (size) {
        *size = ctx->size;
    }
    int prot = (self->mode&ILA_FILE_READ?  PROT_READ  : 0) |
               (self->mode&ILA_FILE_WRITE? PROT_WRITE : 0) |
               (self->mode&ILA_FILE_EXEC?  PROT_EXEC  : 0);
    ctx->contents = mmap(NULL, ctx->size, prot, MAP_SHARED, ctx->fd, 0);
    return ctx->contents;
}

char *strdup(const char*);
ilA_file *ilA_node_stdio_file(const ilA_path *path, enum ilA_file_mode mode)
{
    ilA_file *f = calloc(1, sizeof(ilA_file));
    f->node.type = ILA_NODE_FILE;
    f->node.impl = "stdio";
    f->mode = mode;
    struct stdio_ctx *ctx = calloc(1, sizeof(struct stdio_ctx));
    ctx->path = ilA_path_tochars(path);
    f->node.user = ctx;
    f->node.free = stdio_free;
    f->contents = stdio_contents;
    return f;
}

static void dir_free(ilA_node *self)
{
    ilA_path_free(self->user);
    free(self);
}

static ilA_node* dir_lookup(ilA_dir* self, const ilA_path *path)
{
    struct stat s;
    ilA_path *full = ilA_path_concat(self->node.user, path);
    char *buf = ilA_path_tochars(full);
    if (stat(buf, &s) != 0) {
        return NULL;
    }
    ilA_node* node;
    if (S_ISDIR(s.st_mode)) {
        node = &ilA_node_stdio_dir(full)->node;
    } else {
        node = &ilA_node_stdio_file(full, 0)->node;
    }
    free(buf);
    ilA_path_free(full);
    return node;
}

ilA_dir *ilA_node_stdio_dir(const ilA_path *path)
{
    ilA_dir *d = calloc(1, sizeof(ilA_dir));
    d->node.type = ILA_NODE_DIR;
    d->node.impl = "stdio";
    d->node.free = dir_free;
    d->node.user = ilA_path_copy(path);
    d->lookup = dir_lookup;
    return d;
}

struct union_ctx {
    IL_ARRAY(ilA_dir*,) dirs;
};

static void union_free(ilA_node* self)
{
    struct union_ctx *ctx = self->user;
    unsigned i;
    for (i = 0; i < ctx->dirs.length; i++) {
        ilA_node_unref(ctx->dirs.data[i]);
    }
    IL_FREE(ctx->dirs);
    free(ctx);
    free(self);
}

static ilA_node *union_lookup(ilA_dir* self, const ilA_path *path)
{
    struct union_ctx *ctx = self->node.user;
    ilA_node *res;
    unsigned i;
    for (i = 0; i < ctx->dirs.length; i++) {
        ilA_dir *dir = ctx->dirs.data[i];
        res = ilA_node_lookup(dir, path);
        if (res) {
            return res;
        }
    }
    return NULL;
}

ilA_dir *ilA_node_union(ilA_dir *a, ilA_dir *b)
{
    if (strcmp(a->node.impl, "union") == 0) {
        ilA_node_ref(&b->node);
        IL_APPEND(((struct union_ctx*)a->node.user)->dirs, b);
    } else if (strcmp(b->node.impl, "union") == 0) {
        ilA_node_ref(&a->node);
        IL_APPEND(((struct union_ctx*)b->node.user)->dirs, a);
    } else {
        ilA_node_ref(&a->node);
        ilA_node_ref(&b->node);
    }

    ilA_dir *d = calloc(1, sizeof(ilA_dir));
    d->node.type = ILA_NODE_DIR;
    d->node.impl = "union";
    d->node.free = union_free;
    d->lookup = union_lookup;
    struct union_ctx *ctx = calloc(1, sizeof(struct union_ctx));
    d->node.user = ctx;
    //ctx->dirs = mowgli_list_create();
    IL_APPEND(ctx->dirs, a);
    IL_APPEND(ctx->dirs, b);
    return d;
}

struct prefix_ctx {
    ilA_path *prefix;
    ilA_dir *dir;
};

void prefix_free(ilA_node *self)
{
    struct prefix_ctx *ctx = self->user;
    ilA_path_free(ctx->prefix);
    ilA_node_unref(ctx->dir);
    free(ctx);
    free(self);
}

ilA_node *prefix_lookup(ilA_dir *self, const ilA_path *path)
{
    struct prefix_ctx *ctx = self->node.user;
    ilA_path *p = ilA_path_relativeTo(path, ctx->prefix);
    if (!p) {
        return NULL;
    }
    return ilA_node_lookup(ctx->dir, p);
}

ilA_dir *ilA_node_prefix(ilA_dir *dir, const ilA_path *path)
{
    ilA_dir *d = calloc(1, sizeof(ilA_dir));
    d->node.type = ILA_NODE_DIR;
    d->node.impl = "prefix";
    d->node.free = prefix_free;
    d->lookup = prefix_lookup;
    struct prefix_ctx *ctx = calloc(1, sizeof(struct prefix_ctx));
    d->node.user = ctx;
    ctx->prefix = ilA_path_copy(path);
    ctx->dir = dir;
    ilA_node_ref(dir);
    return d;
}

ilA_node *ilA_node_lookup(ilA_dir* dir, const ilA_path *path)
{
    return dir->lookup(dir, path);
}

void *ilA_node_contents(ilA_file* file, size_t *size)
{
    return file->contents(file, size);
}

