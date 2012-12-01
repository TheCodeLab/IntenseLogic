#ifndef IL_COMMON_LOG_H
#define IL_COMMON_LOG_H

#include <stdio.h>

extern char* strstr(const char* haystack, const char* needle);

FILE *il_logfile;

enum il_logLevel {
  IL_COMMON_LOGFATAL = 0,
  IL_COMMON_LOGERROR,
  IL_COMMON_LOGWARNING,
  IL_COMMON_LOGNOTICE,
  IL_COMMON_LOGDEBUG,
  IL_COMMON_LOGFLOOD
} il_loglevel;

const char* il_loglevel_str[6];
#define il_loglevel_tostring(level) \
  (il_loglevel_str[level])

#define il_prettifyFile(name) \
  (strstr(name, "src/")?strstr(name, "src/"):name)

void il_log_(const char* file, int line, const char *func, int level,
  const char *fmt, ...);

#define il_log(...) il_log_(__FILE__, __LINE__, __func__, \
  __VA_ARGS__)

#endif
