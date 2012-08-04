#include "asset.h"

#include <stdlib.h>
//#include <stdio.h>
#include <libgen.h>
#include <dirent.h>

#include "uthash.h"
#include "IL/il.h"
/* #include "SOIL.h" */

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
    cur = malloc(sizeof(struct SearchPath));
    cur->path = path;
    cur->priority = priority;
    first = cur;
    return;
  }
  
  struct SearchPath *last;
  while (cur->next || cur->priority >= priority) {
    last = cur;
    cur = cur->next;
  }
  
  struct SearchPath *ins = malloc(sizeof(struct SearchPath));
  ins->path = path;
  ins->priority = priority;
  
  last->next = ins;
  ins->next = cur;
}

il_Common_String search_paths(il_Common_String path) {
  struct SearchPath *cur = first;
  char *dir = malloc(path.length+1);
  char *base = malloc(path.length+1);
  strncpy(dir,path.data,path.length);
  strncpy(base,path.data,path.length);
  dir[path.length] = 0;
  base[path.length] = 0;
  dir = dirname(dir);
  base = basename(base);
  while (cur) {
    
    // path ~ '/' ~ dir ~ 0
    char *fullpath = malloc(strlen(dir) + path.length + 2);
    char *p = fullpath;
    
    strncpy(p, path.data, path.length);
    p += path.length;
    
    *p = '/';
    p++;
    
    strncpy(p, dir, strlen(dir));
    p+= strlen(dir);
    
    *p = 0;
    
    struct dirent *entry;
    DIR *dp;
    dp = opendir(fullpath);
    if (dp == NULL) {
      //perror("opendir: Path does not exist or could not be read.");
      return (il_Common_String){0, NULL};
    }
   
    while ((entry = readdir(dp))) {
      if ( 0 == strncmp(base, entry->d_name, strlen(base)>256?256:strlen(base)) ) {
        closedir(dp);
        free(dir);
        free(base);
        return cur->path;
      }
    }
   
    closedir(dp);
    cur = cur->next;
  }
  free(dir);
  free(base);
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
  
  asset = malloc(sizeof(il_Asset_Asset));
  asset->path = path;
  asset->searchdir = res;
  asset->handle = NULL;
  
  asset->fullpath = il_Common_concat(res, il_Common_fromC("/"), path);
  
  HASH_ADD_KEYPTR(hh, assets, path.data, path.length, asset);
  return asset;
}

FILE* il_Asset_getHandle(il_Asset_Asset* asset) { 

  asset->handlerefs++;

  if (asset->handle != NULL) {
    return asset->handle;
  }
  
  asset->handle = fopen(il_Common_toC(asset->fullpath), "r");
  
  return asset->handle;
  
}

void il_Asset_close(il_Asset_Asset* asset) {

  asset->handlerefs--;
  asset->refs--;
  
  if (asset->handlerefs < 0)
    asset->handlerefs = 0;
  if (asset->refs < 0)
    asset->refs = 0;
  
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
  /* return SOIL_load_OGL_texture(il_Common_toC(asset->fullpath), 0, 0, 0); */
  return 1;
}
