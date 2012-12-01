#include "asset.h"

#include <stdlib.h>
//#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "uthash.h"
//#include "IL/il.h"
/* #include "SOIL.h" */
#include "common/log.h"

struct ilA_asset {
    il_string path;
    il_string searchdir;
    il_string fullpath;
    unsigned refs;
    FILE* handle;
    unsigned handlerefs;
    UT_hash_handle hh;
};

ilA_asset* assets = NULL;

struct SearchPath {
    il_string path;
    int priority;
    struct SearchPath *next;
};

struct SearchPath *first;
il_string writedir;

void ilA_init()
{

}

void ilA_setWriteDir(il_string path)
{
    writedir = path;
}

void ilA_registerReadDir(il_string path, int priority)
{
    struct SearchPath *cur = first;
    if (!cur) {
        cur = calloc(1, sizeof(struct SearchPath));
        cur->path = path;
        cur->priority = priority;
        first = cur;
        return;
    }

    struct SearchPath *last;
    while (cur && cur->priority >= priority) {
        last = cur;
        cur = cur->next;
    }

    struct SearchPath *ins = calloc(1, sizeof(struct SearchPath));
    ins->path = path;
    ins->priority = priority;

    last->next = ins;
    ins->next = cur;
}

static il_string search_paths(il_string path)
{
    struct SearchPath *cur = first;
    while (cur) {
        // dir ~ '/' ~ path ~ 0
        char *fullpath = calloc(1, cur->path.length + path.length + 2);
        char *p = fullpath;

        strncpy(p, cur->path.data, cur->path.length);
        p+= cur->path.length;

        *p = '/';
        p++;

        strncpy(p, path.data, path.length);
        p += path.length;

        *p = 0;

        // returns zero on success
        if (!access(fullpath, R_OK|W_OK))
            return cur->path;

        // try again
        cur = cur->next;
    }
    return (il_string) {
        0, NULL
    };
}

ilA_asset* ilA_open(il_string path)
{
    ilA_asset *asset;

    HASH_FIND(hh, assets, path.data, path.length, asset);

    if (asset) {
        asset->refs++;
        return asset;
    }

    il_string res = search_paths(path);

    if (!res.length) {
        res = writedir;
    }

    asset = calloc(1, sizeof(ilA_asset));
    asset->path = path;
    asset->searchdir = res;
    asset->handle = NULL;

    asset->fullpath = il_concat(res, il_fromC("/"), path);

    HASH_ADD_KEYPTR(hh, assets, path.data, path.length, asset);
    return asset;
}

il_string ilA_getPath(ilA_asset* self)
{
    return self->fullpath;
}

FILE* ilA_getHandle(ilA_asset* asset, const char *flags)
{
    return fopen(il_toC(asset->fullpath), flags);
}

il_string ilA_readContents(ilA_asset* asset)
{
    FILE* handle = ilA_getHandle(asset, "r");
    if (!handle) {
        il_log(2, "Could not open file \"%s\": %s (%i)", il_toC(asset->path), strerror(errno), errno);
        return (il_string) {
            0,NULL
        };
    }
    il_string str;

    fseek(handle, 0, SEEK_END); /* Seek to the end of the file */
    str.length = ftell(handle); /* Find out how many bytes into the file we are */
    str.data = (char*)calloc(1, str.length); /* Allocate a buffer for the entire length of the file */
    fseek(handle, 0, SEEK_SET); /* Go back to the beginning of the file */
    fread((char*)str.data, str.length, 1, handle); /* Read the contents of the file in to the buffer */
    fclose(handle); /* Close the file */

    return str;
}

void ilA_close(ilA_asset* asset)
{

    asset->handlerefs--;
    asset->refs--;

    if (asset->handlerefs == 0)
        fclose(asset->handle);
    if (asset->refs == 0) {
        HASH_DELETE(hh, assets, asset);
        free(asset);
    }
}

int ilA_delete(ilA_asset* asset)
{
    if (asset->refs > 1)
        return -1;

    remove(il_toC(asset->fullpath));
    ilA_close(asset);
    return 0;
}

unsigned int ilA_assetToTexture(ilA_asset *asset)
{
    (void)asset;
    /* return SOIL_load_OGL_texture(il_toC(asset->fullpath), 0, 0, 0); */
    return 1;
}
