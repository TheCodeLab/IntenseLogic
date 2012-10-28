#include "asset.h"

#include <stdlib.h>
//#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "uthash.h"
//#include "IL/il.h"
/* #include "SOIL.h" */
#include "common/log.h"

struct il_Asset_Asset {
  il_Common_String path;
  il_Common_String searchdir;
  il_Common_String fullpath;
  unsigned refs;
  FILE* handle;
  unsigned handlerefs;
  UT_hash_handle hh;
};

il_Asset_Asset* assets = NULL;

struct SearchPath {
  il_Common_String path;
  int priority;
  struct SearchPath *next;
};

struct SearchPath *first;
il_Common_String writedir;

void il_Asset_init() {

}

void il_Asset_setWriteDir(il_Common_String path) {
  writedir = path;
}

void il_Asset_registerReadDir(il_Common_String path, int priority) {
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

static il_Common_String search_paths(il_Common_String path) {
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
  return (il_Common_String){0, NULL};
}

il_Asset_Asset* il_Asset_open(il_Common_String path) {
  il_Asset_Asset *asset;
  
  HASH_FIND(hh, assets, path.data, path.length, asset);
  
  if (asset) {
    asset->refs++;
    return asset;
  }
  
  il_Common_String res = search_paths(path);
  
  if (!res.length) {
    res = writedir;
  }
  
  asset = calloc(1, sizeof(il_Asset_Asset));
  asset->path = path;
  asset->searchdir = res;
  asset->handle = NULL;
  
  asset->fullpath = il_Common_concat(res, il_Common_fromC("/"), path);
  
  HASH_ADD_KEYPTR(hh, assets, path.data, path.length, asset);
  return asset;
}

il_Common_String il_Asset_getPath(il_Asset_Asset* self) {
  return self->fullpath;
}

FILE* il_Asset_getHandle(il_Asset_Asset* asset, const char *flags) {
  return fopen(il_Common_toC(asset->fullpath), flags);
}

il_Common_String il_Asset_readContents(il_Asset_Asset* asset) {
  FILE* handle = il_Asset_getHandle(asset, "r");
  if (!handle) {
    il_Common_log(2, "Could not open file \"%s\": %s (%i)", il_Common_toC(asset->path), strerror(errno), errno);
    return (il_Common_String){0,NULL};
  }
  il_Common_String str;
  
  fseek(handle, 0, SEEK_END); /* Seek to the end of the file */
  str.length = ftell(handle); /* Find out how many bytes into the file we are */
  str.data = (char*)calloc(1, str.length); /* Allocate a buffer for the entire length of the file */
  fseek(handle, 0, SEEK_SET); /* Go back to the beginning of the file */
  fread((char*)str.data, str.length, 1, handle); /* Read the contents of the file in to the buffer */
  fclose(handle); /* Close the file */
  
  return str;
}

void il_Asset_close(il_Asset_Asset* asset) {

  asset->handlerefs--;
  asset->refs--;
  
  if (asset->handlerefs == 0)
    fclose(asset->handle);
  if (asset->refs == 0) {
    HASH_DELETE(hh, assets, asset);
    free(asset);
  }
}

int il_Asset_delete(il_Asset_Asset* asset) {
  if (asset->refs > 1)
    return -1;
  
  remove(il_Common_toC(asset->fullpath));
  il_Asset_close(asset);
  return 0;
}

unsigned int il_Asset_assetToTexture(il_Asset_Asset *asset) {
  (void)asset;
  /* return SOIL_load_OGL_texture(il_Common_toC(asset->fullpath), 0, 0, 0); */
  return 1;
}
