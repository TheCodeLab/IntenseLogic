#include "string.h"

#include <stdlib.h>
#include <stdarg.h>

char *il_Common_toC(const il_Common_String s) {
  char *z = malloc(s.length+1);
  strncpy(z, s.data, s.length);
  z[s.length] = (char)0;
  return z;
}

il_Common_String il_Common_concatfunc(const il_Common_String s, ...) {

  il_Common_String str;

  va_list va;
  
  il_Common_String arg = s;
  va_start(va, s);
  while (arg.length) {
    str.length+=arg.length;
    arg = va_arg(va, il_Common_String);
  }
  va_end(va);
  
  str.data = (char*)malloc(str.length);
  
  va_start(va, s);
  char *p = str.data;
  while (arg.length) {
    strncpy(p, arg.data, arg.length);
    p += arg.length;
  }
  va_end(va);
  
  return str;
}
