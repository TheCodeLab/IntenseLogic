#ifndef IL_COMMON_STRING_H
#define IL_COMMON_STRING_H

#include <string.h>

typedef struct il_string {
  size_t length;
  const char *data;
} il_string;
#define il_String il_string

il_string il_CtoS(const char * s, int len);
#define il_fromC(s) (il_CtoS(s, -1))
#define il_l(s) (il_CtoS(s, -1))

const char *il_StoC(il_string s);
#define il_toC il_StoC

#define il_concat(...) (il_concatfunc(__VA_ARGS__, (il_string){0,NULL}))
il_string il_concatfunc(il_string s, ...);

int il_strcmp(il_string a, il_string b);
size_t il_len(il_string s);
il_string il_strdup(il_string s);

#endif
