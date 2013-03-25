#include "node.h"

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

ilA_node *ilA_node_ref(ilA_node* node) 
{
    node->refs++;
    return node;
}

void ilA_node_unref(ilA_node* node)
{
    node->refs--;
    if (node->refs == 0) {
        node->free(node);
    }
}

struct stdio_ctx {
    int fd;
    void *contents;
    size_t size;
};

static void stdio_free(ilA_node *self)
{
    struct stdio_ctx *ctx = self->user;
    munmap(ctx->contents, ctx->size);
    close(ctx->fd);
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

ilA_file *ilA_node_stdio_file(const char *path, enum ilA_file_mode mode)
{
    ilA_file *f = calloc(1, sizeof(ilA_file));
    f->node.type = ILA_NODE_FILE;
    f->mode = mode;
    struct stdio_ctx *ctx = calloc(1, sizeof(struct stdio_ctx));
    f->node.user = ctx;
    int oflag = (mode&ILA_FILE_READ && mode&ILA_FILE_WRITE? O_RDWR :
                    (mode&ILA_FILE_READ? O_RDONLY : 0) |
                    (mode&ILA_FILE_WRITE? O_WRONLY : 0));
    ctx->fd = open(path, oflag);
    f->node.free = stdio_free;
    f->contents = stdio_contents;
    return f;
}

ilA_dir *ilA_node_stdio_dir(const char *path);

ilA_dir *ilA_node_union(ilA_dir *a, ilA_dir *b);

ilA_node *ilA_node_lookup(const char *path, ilA_dir* dir)
{
    return dir->lookup(dir, path);
}

