#ifndef IL_COMMON_LOG_H
#define IL_COMMON_LOG_H

#include <stdio.h>

FILE *il_Common_logfile;

enum il_Common_LogLevel {
  IL_COMMON_LOGFATAL = 0,
  IL_COMMON_LOGERROR,
  IL_COMMON_LOGWARNING,
  IL_COMMON_LOGNOTICE,
  IL_COMMON_LOGDEBUG,
  IL_COMMON_LOGFLOOD
} il_Common_loglevel;

const char* il_Common_loglevel_str[6];
#define il_Common_loglevel_tostring(level) \
  (il_Common_loglevel_str[level])

#define il_Common_log(level, format, ...) \
  do {                                    \
    if ((level) <= il_Common_loglevel) {  \
      fprintf ( il_Common_logfile,        \
                ("%s:%i (%s) %s: " format), \
                __FILE__,                 \
                __LINE__,                 \
                __func__,                 \
                il_Common_loglevel_tostring(level), \
                ##__VA_ARGS__ );          \
    }                                     \
  } while (0)

#endif
