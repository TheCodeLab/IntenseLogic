#include "node.h"

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>

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
ilA_file *ilA_node_stdio_file(const char *path, enum ilA_file_mode mode)
{
    ilA_file *f = calloc(1, sizeof(ilA_file));
    f->node.type = ILA_NODE_FILE;
    f->mode = mode;
    struct stdio_ctx *ctx = calloc(1, sizeof(struct stdio_ctx));
    ctx->path = strdup(path);
    f->node.user = ctx;
    f->node.free = stdio_free;
    f->contents = stdio_contents;
    return f;
}

static void dir_free(ilA_node *self)
{
    free(self->user);
    free(self);
}

static ilA_node* dir_lookup(ilA_dir* self, const char *path)
{
    struct stat s;
    char *buf = malloc(strlen(path) + strlen(self->node.user) + 2);
    strcpy(buf, self->node.user);
    strcat(buf, "/");
    strcat(buf, path);
    if (stat(path, &s) != 0) {
        return NULL;
    }
    ilA_node* node;
    if (S_ISDIR(s.st_mode)) {
        node = &ilA_node_stdio_dir(buf)->node;
    } else {
        node = &ilA_node_stdio_file(buf, 0)->node;
    }
    free(buf);
    return node;
}

ilA_dir *ilA_node_stdio_dir(const char *path)
{
    ilA_dir *d = calloc(1, sizeof(ilA_dir));
    d->node.type = ILA_NODE_DIR;
    d->node.free = dir_free;
    d->node.user = strdup(path);
    d->lookup = dir_lookup;
    return d;
}

ilA_dir *ilA_node_union(ilA_dir *a, ilA_dir *b);

ilA_node *ilA_node_lookup(const char *path, ilA_dir* dir)
{
    return dir->lookup(dir, path);
}

