#ifndef ILU_LOG_H
#define ILU_LOG_H

#include <string.h>

const char *il_log_prefixes[5];

#define il_prettifyFile(name) \
  (strstr(name, "src/")?strstr(name, "src/"):name)

#define il_log_real(file, line, func, lvl, fmt, ...) \
    if (il_can_log(file, lvl)) { \
        mowgli_log_prefix_real(file, line, func, prefixes[lvl], fmt, __VA_ARGS__); \
    }

#define il_debug(fmt, ...)      il_log_real(il_prettifyFile(__FILE__), __LINE__, __func__, 4, fmt, __VA_ARGS__)
#define il_log(fmt, ...)        il_log_real(il_prettifyFile(__FILE__), __LINE__, __func__, 3, fmt, __VA_ARGS__)
#define il_warning(fmt, ...)    il_log_real(il_prettifyFile(__FILE__), __LINE__, __func__, 2, fmt, __VA_ARGS__)
#define il_error(fmt, ...)      il_log_real(il_prettifyFile(__FILE__), __LINE__, __func__, 1, fmt, __VA_ARGS__)
#define il_fatal(fmt, ...) \
    do { \
        il_log_real(il_prettifyFile(__FILE__), __LINE__, __func__, 0, fmt, __VA_ARGS__); \
        abort(); \
    } while(0)

int il_can_log(const char *file, int level);

void il_log_toggle(const char *module, int level);

#endif

