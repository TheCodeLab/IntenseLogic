#ifndef IL_COMMON_STRING_H
#define IL_COMMON_STRING_H

#include <string.h>

typedef struct il_String {
  size_t length;
  const char *data;
} il_String;
#define il_Common_String il_String

il_String il_CtoS(const char * s, int len);
#define il_Common_fromC(s) (il_CtoS(s, -1))
#define il_l(s) (il_CtoS(s, -1))

const char *il_StoC(il_String s);
#define il_Common_toC il_StoC

#define il_concat(...) (il_concatfunc(__VA_ARGS__, (il_Common_String){0,NULL}))
#define il_Common_concat il_concat
il_String il_concatfunc(il_String s, ...);

int il_strcmp(il_String a, il_String b);

#define il_len(s) (strnlen(s.data, s.length))

#endif
