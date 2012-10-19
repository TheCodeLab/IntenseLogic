#include "string.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

size_t strnlen(const char *s, size_t maxlen);

il_String il_CtoS(const char * s, int len) {
  if (len < 0) return (il_String){strlen(s), s};
  return (il_String){strnlen(s, len), s};
}

const char *il_StoC(il_String s) {
  size_t len = strnlen(s.data, s.length);
  if (len < s.length) return s.data; // s already has a null terminator
  char * z = calloc(1, len+1);
  memcpy(z, s.data, len);
  z[len] = 0;
  return z;
}

il_String il_concatfunc(il_String s, ...) {

  il_String str = (il_String){0, NULL};

  va_list va;
  
  il_String arg = s;
  va_start(va, s);
  while (arg.length) {
    str.length+=arg.length;
    arg = va_arg(va, il_String);
  }
  va_end(va);
  
  str.data = (char*)calloc(1, str.length);
  
  va_start(va, s);
  char *p = (char*)str.data;
  arg = s;
  while (arg.length) {
    strncpy(p, arg.data, arg.length);
    p += arg.length;
    arg = va_arg(va, il_String);
  }
  va_end(va);
  
  return str;
}

int il_strcmp(il_String a, il_String b) {
  if (a.length < b.length) return -1;
  return strncmp(a.data, b.data, b.length);
}
