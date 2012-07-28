#ifndef IL_COMMON_STRING_H
#define IL_COMMON_STRING_H

#include <string.h>

typedef struct il_Common_String {
  unsigned length;
  char *data;
} il_Common_String;

typedef struct il_Common_OctetString {
  unsigned length;
  void *data;
} il_Common_OctetString;

#define il_Common_fromC(s) ((il_Common_String){strlen(s), s})

char *il_Common_toC(const il_Common_String s);

#define il_Common_concat(...) (il_Common_concatfunc(__VA_ARGS__, (il_Common_String){0,NULL}))

il_Common_String il_Common_concatfunc(const il_Common_String s, ...);

#endif
