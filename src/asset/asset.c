#include "asset.h"

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "util/uthash.h"
#include "util/log.h"
#include "asset/image.h"

struct ilA_asset {
    il_string *path;
    il_string *searchdir;
    il_string *fullpath;
    unsigned refs;
    FILE* handle;
    unsigned handlerefs;
    UT_hash_handle hh;
};

ilA_asset* assets = NULL;

struct SearchPath {
    il_string *path;
    int priority;
    struct SearchPath *next;
};

struct SearchPath *first;
il_string *writedir;

void ilA_union_init();
void ilA_stdiofile_init();
void ilA_stdiodir_init();
char *strdup(const char*);
char *strtok_r(char *str, const char *delim, char **saveptr);
int il_bootstrap(int argc, char **argv)
{
    (void)argc, (void)argv;
    // read environment variables
    char *path = getenv("IL_PATH");
    if (path) {
        char *saveptr = NULL;
        char *str = strdup(path);
        char *token;

        token = strtok_r(str, ":", &saveptr);
        while(token) {
            ilA_registerReadDir(il_string_new(token, strlen(token)), 3);
            token = strtok_r(NULL, ":", &saveptr);
        }
    } else {
        // reasonable defaults
        ilA_registerReadDir(il_string_new(".",       -1), 4);
        ilA_registerReadDir(il_string_new("config",  -1), 4);
        ilA_registerReadDir(il_string_new("shaders", -1), 4);
    }

    // TODO: Parse command line flags
    /*if (args.path){
        ilA_registerReadDir(il_string_new(args.path, strlen(args.path)), 1);
    }*/

    ilA_union_init();
    ilA_stdiofile_init();
    ilA_stdiodir_init();

    return 1;
}

void ilA_setWriteDir(il_string *path)
{
    writedir = il_string_ref(path);
}

void ilA_registerReadDir(il_string *path, int priority)
{
    il_string_ref(path);
    struct SearchPath *cur = first;
    if (!cur) {
        cur = calloc(1, sizeof(struct SearchPath));
        cur->path = path;
        cur->priority = priority;
        first = cur;
        return;
    }

    struct SearchPath *last = NULL;
    while (cur && cur->priority >= priority) {
        last = cur;
        cur = cur->next;
    }

    struct SearchPath *ins = calloc(1, sizeof(struct SearchPath));
    ins->path = path;
    ins->priority = priority;

    if (last) {
        last->next = ins;
    }
    ins->next = cur;
}

size_t strnlen(const char*,size_t);
static il_string *search_paths(il_string *path)
{
    struct SearchPath *cur = first;
    while (cur) {
        // dir ~ '/' ~ path ~ 0
        char *fullpath = calloc(1, cur->path->length + path->length + 2);
        char *p = fullpath;

        strncpy(p, cur->path->data, cur->path->length);
        p+= strnlen(cur->path->data, cur->path->length);

        *p = '/';
        p++;

        strncpy(p, path->data, path->length);
        p += strnlen(path->data, path->length);

        *p = 0;

        // returns zero on success
        if (!access(fullpath, R_OK|W_OK)) {
            free(fullpath);
            return il_string_ref(cur->path);
        }

        // try again
        cur = cur->next;
    }
    return NULL;
}

ilA_asset* ilA_open(il_string *path)
{
    ilA_asset *asset;

    HASH_FIND(hh, assets, path->data, path->length, asset);

    if (asset) {
        asset->refs++;
        return asset;
    }

    il_string *res = search_paths(path);

    if (!res) {
        res = writedir;
    }

    asset = calloc(1, sizeof(ilA_asset));
    asset->path = il_string_ref(path);
    asset->searchdir = res;
    asset->handle = NULL;

    const char *cres = il_StoC(res), *cpath = il_StoC(path);
    if (!cres) {
        asset->fullpath = il_string_ref(path);
    } else {
        if (!cpath) {
            il_error("cpath is null");
            return NULL;
        }
        asset->fullpath = il_string_format("%s/%s", cres, cpath);
    }

    il_string_unref(res);

    HASH_ADD_KEYPTR(hh, assets, path->data, path->length, asset);
    return asset;
}

ilA_asset* ilA_ref(void* ptr) 
{
    ilA_asset* asset = ptr;
    asset->refs++;
    return asset;
}

void ilA_unref(void* ptr) 
{
    ilA_asset* asset = ptr;
    asset->refs--;
}

il_string *ilA_getPath(ilA_asset* self)
{
    return il_string_ref(self->fullpath);
}

FILE* ilA_getHandle(ilA_asset* asset, const char *flags)
{
    const char *cpath = il_StoC(asset->fullpath);
    if (!cpath) {
        il_error("cpath is null");
        return NULL;
    }
    return fopen(cpath, flags);
}

il_string *ilA_readContents(ilA_asset* asset)
{
    FILE* handle = ilA_getHandle(asset, "r");
    if (!handle) {
        il_error("Could not open file \"%s\": %s (%i)", il_StoC(asset->path), strerror(errno), errno);
        return NULL;
    }
    il_string *str;
    size_t len;

    fseek(handle, 0, SEEK_END); /* Seek to the end of the file */
    len = ftell(handle); /* Find out how many bytes into the file we are */
    char buf[len];
    fseek(handle, 0, SEEK_SET); /* Go back to the beginning of the file */
    fread(buf, len, 1, handle); /* Read the contents of the file in to the buffer */
    fclose(handle); /* Close the file */
    str = il_string_bin(buf, len);

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

    remove(il_StoC(asset->fullpath));
    ilA_close(asset);
    return 0;
}

