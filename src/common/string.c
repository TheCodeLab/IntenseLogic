#include "string.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

size_t strnlen(const char *s, size_t maxlen);

#ifdef WIN32
size_t strnlen(const char *str, size_t maxlen)
{
  const char *char_ptr, *end_ptr = str + maxlen;
  const unsigned long int *longword_ptr;
  unsigned long int longword, himagic, lomagic;

  if (maxlen == 0)
    return 0;

  if (end_ptr < str, 0)
    end_ptr = (const char *) ~0UL;

  for (char_ptr = str; ; ++char_ptr) {
    if (*char_ptr == '\0') {
      if (char_ptr > end_ptr)
        char_ptr = end_ptr;
      return char_ptr - str;
    }
  }
  return end_ptr - str;
}
#endif

il_String il_CtoS(const char * s, int len) {
  if (len < 0) return (il_String){strlen(s), s};
  return (il_String){strnlen(s, len)+1, s};
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
    str.length+=strnlen(arg.data,arg.length);
    arg = va_arg(va, il_String);
  }
  va_end(va);
  
  str.data = (char*)calloc(1, str.length);
  
  va_start(va, s);
  char *p = (char*)str.data;
  arg = s;
  while (arg.length) {
    strncpy(p, arg.data, strnlen(arg.data,arg.length));
    p += strnlen(arg.data,arg.length);
    arg = va_arg(va, il_String);
  }
  va_end(va);
  
  return str;
}

int il_strcmp(il_String a, il_String b) {
  size_t alen = strnlen(a.data,a.length);
  size_t blen = strnlen(b.data,b.length);
  if (alen < blen) return -1;
  return strncmp(a.data, b.data, blen);
}
