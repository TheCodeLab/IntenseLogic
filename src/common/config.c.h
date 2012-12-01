#include "config.h"

#include <string.h>
#include <ini.h>

#define FN_(prefix, name) prefix##_##name
#define FN(prefix,name) FN_(prefix,name)

int FN(IL_CONFIG_TYPE,handle)(void *user, const char *section, const char *name, const char *value) {

  struct IL_CONFIG_TYPE *cfg = (struct IL_CONFIG_TYPE*)user;

  if (0) ;
  #define CFG(s, n, default) else if (strcmp(section, #s)==0 && \
      strcmp(name, #n)==0) cfg->s##_##n = strdup(value);
  #include IL_CONFIG_DEFS
  #undef CFG

  return 1;
}

void FN(IL_CONFIG_TYPE,defaults)(struct IL_CONFIG_TYPE * cfg) {
  if (!cfg) return;
  *cfg = (struct IL_CONFIG_TYPE) {
    #define CFG(s, n, default) default,
    #include IL_CONFIG_DEFS
    #undef CFG
  };
}

void FN(IL_CONFIG_TYPE,parse)(const char *file, struct IL_CONFIG_TYPE * cfg) {
  if (!cfg) return;
  ini_parse(file, FN(IL_CONFIG_TYPE,handle), cfg);
}

#undef FN_
#undef FN
