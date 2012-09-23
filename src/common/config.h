#include <string.h>
#include <ini.h>

char * strdup(const char*);

typedef struct IL_CONFIG_TYPE {
  #define CFG(s, n, def) const char *s##_##n;
  #include IL_CONFIG_DEFS
  #undef CFG
} IL_CONFIG_TYPE;

#define FN_(prefix, name) prefix##_##name
#define FN(prefix,name) FN_(prefix,name)

int FN(IL_CONFIG_TYPE,handle)(void *user, const char *section, const char *name, const char *value);

void FN(IL_CONFIG_TYPE,defaults)(struct IL_CONFIG_TYPE * cfg);

void FN(IL_CONFIG_TYPE,parse)(const char *file, struct IL_CONFIG_TYPE * cfg);

#undef FN_
#undef FN
