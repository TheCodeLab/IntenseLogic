#include "mtl.h"

#include <stdio.h>
#include <stdlib.h>

#include "util/log.h"
#include "asset/image.h"

char *strsep(char **stringp, const char *delim);

static int parse_line(ilA_mtl *mtl, char *line, char *error)
{
    char *word;
    int col = 0;
    ilA_mtl *cur = mtl->cur;

#define next_word strsep(&line, " ")
    word = next_word;
    if (strcmp(word, "newmtl") == 0) {
        char *name = strdup(next_word);
        ilA_mtl *new = calloc(1, sizeof(ilA_mtl));
        new->name = name;
        new->id = mtl->id++;
        new->ambient[3] = 255;
        unsigned char diffuse[4] = {127, 127, 127, 255};
        memcpy(new->diffuse, diffuse, sizeof(diffuse));
        unsigned char specular[4] = {127, 127, 127, 96};
        memcpy(new->specular, specular, sizeof(specular));
        HASH_ADD_KEYPTR(hh, mtl, name, strlen(name), new);
        mtl->cur = new;
    } else if (strcmp(word, "Ka") == 0) {
        cur->ambient[0] = strtof(next_word, NULL) * 255;
        cur->ambient[1] = strtof(next_word, NULL) * 255;
        cur->ambient[2] = strtof(next_word, NULL) * 255;
    } else if (strcmp(word, "Kd") == 0) {
        cur->diffuse[0] = strtof(next_word, NULL) * 255;
        cur->diffuse[1] = strtof(next_word, NULL) * 255;
        cur->diffuse[2] = strtof(next_word, NULL) * 255;
    } else if (strcmp(word, "Ks") == 0) {
        cur->specular[0] = strtof(next_word, NULL) * 255;
        cur->specular[1] = strtof(next_word, NULL) * 255;
        cur->specular[2] = strtof(next_word, NULL) * 255;
    } else if (strcmp(word, "Ns") == 0) {
        cur->specular[3] = strtof(next_word, NULL);
    } else if (strcmp(word, "Tr") == 0 || strcmp(word, "d") == 0) {
        cur->transparency = strtof(next_word, NULL);
    } else if (strcmp(word, "#") != 0 && *word != 0) {
        snprintf(error, 1024, "Unknown command \"%s\"", word);
        col = 1;
    }
#undef next
    return col;
}

char *strndup(const char*, size_t);
ilA_mtl *ilA_mesh_parseMtl(ilA_mtl *mtl, const char *filename, const char *data, size_t length)
{
    char *str = strndup(data, length), *saveptr = str, *ptr, error[1024], colstr[8];
    int line = 0, col;

    if (!mtl) {
        mtl = calloc(1, sizeof(ilA_mtl));
    }
    if (!mtl->hh.tbl) {
        HASH_MAKE_TABLE(hh, mtl);
    }
    while ((ptr = strsep(&saveptr, "\n"))) {
        line++;
        col = parse_line(mtl, ptr, error);
        if (col > 0) {
            snprintf(colstr, 8, "%i", col);
            il_log_real(filename, line, colstr, 2, "%s", error);
        }
    }
    free(str);
    return mtl;
}

void ilA_mtl_free(ilA_mtl *self)
{
    ilA_mtl *node, *tmp;
    HASH_ITER(hh, self, node, tmp) {
        HASH_DELETE(hh, self, node);
        if (node->name) {
            free(node->name);
        }
        if (node->diffuse_map.data) {
            ilA_img_free(node->diffuse_map);
        }
        if (node->specular_map.data) {
            ilA_img_free(node->specular_map);
        }
        if (node->specular_highlight_map.data) {
            ilA_img_free(node->specular_highlight_map);
        }
        free(node);
    }
    free(self);
}
