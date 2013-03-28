#ifndef ILU_LOG_H
#define ILU_LOG_H

#include <string.h>
//#include <mowgli.h>

extern void mowgli_log_prefix_real(const char *file, int line, const char *func, const char *prefix, const char *fmt, ...);

const char *il_log_prefixes[5];

#define il_prettifyFile(name) \
  (strstr(name, "src/")?strstr(name, "src/")+4:name)

#define il_log_real(file, line, func, lvl, ...) \
    if (il_can_log(file, lvl)) { \
        mowgli_log_prefix_real(file, line, func, il_log_prefixes[lvl], __VA_ARGS__); \
    }

#define il_debug(...)      il_log_real(il_prettifyFile(__FILE__), __LINE__, __func__, 4, __VA_ARGS__)
#define il_log(...)        il_log_real(il_prettifyFile(__FILE__), __LINE__, __func__, 3, __VA_ARGS__)
#define il_warning(...)    il_log_real(il_prettifyFile(__FILE__), __LINE__, __func__, 2, __VA_ARGS__)
#define il_error(...)      il_log_real(il_prettifyFile(__FILE__), __LINE__, __func__, 1, __VA_ARGS__)
#define il_fatal(...) \
    do { \
        il_log_real(il_prettifyFile(__FILE__), __LINE__, __func__, 0, __VA_ARGS__); \
        abort(); \
    } while(0)

int il_can_log(const char *file, int level);

void il_log_toggle(const char *module, int level);

#endif

