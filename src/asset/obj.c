#include "mesh.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "util/log.h"
#include "asset/mtl.h"

char *strsep(char **stringp, const char *delim);

static void *resize(void *src, size_t size, size_t newsize)
{
    void *dst = calloc(1, newsize);
    if (src) {
        memcpy(dst, src, size);
    }
    free(src);
    return dst;
}

struct face {
    size_t num;
    int vert[4];
    int tex[4];
    int norm[4];
    ilA_mtl *mtl;
};

struct obj {
    size_t num_vertex, vertex_cap;
    float (*vertex)[4];
    size_t num_texcoord, texcoord_cap;
    float (*texcoord)[4];
    size_t num_normal, normal_cap;
    float (*normal)[4];
    size_t num_face, face_cap;
    struct face *face;
    ilA_mtl mtl, *cur;
    ilA_fs *fs;
};

static int parse_line(struct obj *obj, char *line, char *error)
{
    char *saveptr = line, *word;
    float fvec[4] = {0, 0, 0, 1};
    //unsigned char cvec[4] = {0};
    int i, col = -1;

#define next_word strsep(&saveptr, " ")
    word = next_word;
    if (strcmp(word, "v") == 0) {
        for (i = 0, word = next_word; word && i < 4; word = next_word, i++) {
            errno = 0;
            fvec[i] = strtof(word, NULL);
            if (errno) {
                snprintf(error, 1024, "Failed to parse component %i of vector: %s", i+1, strerror(errno));
                col = word - line;
            }
        }
        if (obj->num_vertex >= obj->vertex_cap) {
            obj->vertex = resize(obj->vertex, obj->num_vertex * sizeof(float) * 4, obj->vertex_cap * 2 * sizeof(float) * 4);
            obj->vertex_cap *= 2;
        }
        memcpy(obj->vertex[obj->num_vertex++], fvec, sizeof(float) * 4);
    } else if (strcmp(word, "vt") == 0) {
        for (i = 0, word = next_word; word && i < 3; word = next_word, i++) {
            errno = 0;
            fvec[i] = strtof(word, NULL);
            if (errno) {
                snprintf(error, 1024, "Failed to parse component %i of texcoord: %s", i+1, strerror(errno));
                col = word - line;
            }
        }
        if (obj->num_texcoord >= obj->texcoord_cap) {
            obj->texcoord = resize(obj->texcoord, obj->num_texcoord * sizeof(float) * 4, obj->texcoord_cap * 2 * sizeof(float) * 4);
            obj->texcoord_cap *= 2;
        }
        memcpy(obj->texcoord[obj->num_texcoord++], fvec, sizeof(float) * 4);
    } else if (strcmp(word, "vn") == 0) {
        for (i = 0, word = next_word; word && i < 3; word = next_word, i++) {
            errno = 0;
            fvec[i] = strtof(word, NULL);
            if (errno) {
                snprintf(error, 1024, "Failed to parse component %i of normal: %s", i+1, strerror(errno));
                col = word - line;
            }
        }
        if (obj->num_normal >= obj->normal_cap) {
            obj->normal = resize(obj->normal, obj->num_normal * sizeof(float) * 4, obj->texcoord_cap * 2 * sizeof(float) * 4);
            obj->normal_cap *= 2;
        }
        memcpy(obj->normal[obj->num_normal++], fvec, sizeof(float) * 4);
    } else if (strcmp(word, "f") == 0) {
        struct face face;
        memset(&face, 0, sizeof(struct face));
        for (i = 0, word = next_word; word && i < 4; word = next_word, i++) {
            face.vert[i] = -1;
            face.tex[i] = -1;
            face.norm[i] = -1;
            char *num = strsep(&word, "/");
            if (!num) {
                continue;
            }
            errno = 0;
            int id = strtol(num, NULL, 10);
            if (errno) {
                snprintf(error, 1024, "Failed to parse vertex component %i of face: %s", i+1, strerror(errno));
                col = num - line;
            }
            if (id < 0) {
                id += obj->num_vertex;
            }
            face.vert[i] = id - 1;
            errno = 0;
            num = strsep(&word, "/");
            if (!num) {
                continue;
            }
            id = strtol(num, NULL, 10);
            if (errno) {
                snprintf(error, 1024, "Failed to parse texcoord component %i of face: %s", i, strerror(errno));
                col = num - line;
            }
            if (id < 0) {
                id += obj->num_texcoord;
            }
            face.tex[i] = id - 1;
            errno = 0;
            num = strsep(&word, "/");
            if (!num) {
                continue;
            }
            id = strtol(num, NULL, 10);
            if (errno) {
                snprintf(error, 1024, "Failed to parse normal component %i of face: %s", i, strerror(errno));
                col = num - line;
            }
            if (id < 0) {
                id += obj->num_normal;
            }
            face.norm[i] = id - 1;
        }
        face.mtl = obj->cur;
        face.num = i;
        if (obj->num_face >= obj->face_cap) {
            obj->face = resize(obj->face, obj->num_face * sizeof(struct face), obj->face_cap * 2 * sizeof(struct face));
            obj->face_cap *= 2;
        }
        memcpy(obj->face + obj->num_face++, &face, sizeof(struct face));
    } else if (strcmp(word, "mtllib") == 0) {
        char *filename = next_word;
        ilA_map map;
        if (!ilA_mapfile(obj->fs, &map, ILA_READ, filename, -1)) {
            ilA_printerror(&map.err);
            return 0;
        }
        ilA_mesh_parseMtl(&obj->mtl, filename, map.data, map.size);
        ilA_unmapfile(&map);
    } else if (strcmp(word, "usemtl") == 0) {
        char *name = next_word;
        HASH_FIND(hh, &obj->mtl, name, strlen(name), obj->cur);
    } else if (strcmp(word, "#") != 0) {
        snprintf(error, 1024, "Unknown command \"%s\".", word);
        col = word - line;
    }
#undef next_word

    return col;
}

ilA_mesh *ilA_mesh_parseObj(ilA_fs *fs, const char *filename, const char *data, size_t length)
{
    struct obj *obj = calloc(1, sizeof(struct obj));
    obj->fs = fs;
    ilA_mesh *mesh;
    char *str = malloc(length+1), *saveptr = str, *ptr, error[1024], col_str[8];
    strncpy(str, data, length);
    str[length] = 0;
    int line = 0, col;
    size_t i, j, v = 0, num_vertices = 0;

    obj->vertex = calloc(sizeof(float) * 4, 1000);
    obj->texcoord = calloc(sizeof(float) * 4, 1000);
    obj->normal = calloc(sizeof(float) * 4, 1000);
    obj->face = calloc(sizeof(struct face), 100);
    obj->vertex_cap = obj->texcoord_cap = obj->normal_cap = 1000;
    obj->face_cap = 100;
    while ((ptr = strsep(&saveptr, "\n"))) {
        line++;
        col = parse_line(obj, ptr, error);
        if (col >= 0) {
            snprintf(col_str, 8, "%i", col);
            il_log_real(filename, line, col_str, 2, "%s", error);
        }
    }
    free(str);
    for (i = 0; i < obj->num_face; i++) {
        num_vertices += obj->face[i].num == 4? 6 : obj->face[i].num;
    }
    mesh = ilA_mesh_new(ILA_MESH_POSITION|ILA_MESH_TEXCOORD|ILA_MESH_NORMAL|ILA_MESH_DIFFUSE|ILA_MESH_SPECULAR, num_vertices);
    mesh->mode = ILA_MESH_TRIANGLES;
    for (i = 0; i < obj->num_face; i++) {
        struct face *face = obj->face + i;
        static int quad_to_tri[] = {
            0, 1, 2, 2, 3, 0
        };
        for (j = 0; j < (face->num == 4? 6 : face->num); j++) {
            int idx = face->num == 4? quad_to_tri[j] : j;
            int vert = face->vert[idx];
            if (vert < 0 || vert > (int)obj->num_vertex) {
                if (vert != -1) {
                    il_error("Invalid vertex ID %i", vert);
                }
            } else {
                memcpy(mesh->position[v], obj->vertex[vert], sizeof(float) * 4);
            }
            int tex = face->tex[idx];
            if (tex < 0 || tex > (int)obj->num_texcoord) {
                if (tex != -1) {
                    il_error("Invalid texcoord ID %i", tex);
                }
            } else {
                memcpy(mesh->texcoord[v], obj->texcoord[face->tex[ idx]], sizeof(float) * 4);
            }
            int norm = face->norm[idx];
            if (norm < 0 || norm > (int)obj->num_normal) {
                if (norm != -1) {
                    il_error("Invalid normal ID %i", norm);
                }
            } else {
                memcpy(mesh->normal[v],   obj->normal[  face->norm[idx]], sizeof(float) * 4);
            }
            if (face->mtl) {
                //memcpy(mesh->ambient[v],  face->mtl->ambient, sizeof(unsigned char) * 4);
                memcpy(mesh->diffuse[v],  face->mtl->diffuse, sizeof(unsigned char) * 4);
                memcpy(mesh->specular[v], face->mtl->specular, sizeof(unsigned char) * 4);
            }
            v++;
        }
    }
    return mesh;
}
