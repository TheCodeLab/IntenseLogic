/** @file log.h
 * @brief Responsible for logging facilities and limiting flooding from irrelevant modules while debugging
 */

#ifndef ILU_LOG_H
#define ILU_LOG_H

#include <string.h>
#include <stdio.h>

extern const char *il_log_prefixes[5];

/** A macro for shortening file names */
#define il_prettifyFile(name) \
  (strstr(name, "src/")?strstr(name, "src/")+4:name)

#define il_log_raw(msg) \
    fprintf(stderr, "%s\n", msg);

/** The underlying macro for logging */
#define il_log_real(file, line, func, lvl, ...) \
    if (il_can_log(file, lvl)) { \
        char _buf[4096]; \
        snprintf(_buf, 4096, __VA_ARGS__); \
        char _msg[4096]; \
        snprintf(_msg, 4096, "(%s:%i %s) %s%s", file, line, func, il_log_prefixes[lvl], _buf);\
        il_log_raw(_msg); \
    }

/** For information that would only be useful to a programmer */
#define il_debug(...)      il_log_real(il_prettifyFile(__FILE__), __LINE__, __func__, 4, __VA_ARGS__)
/** Informative messages */
#define il_log(...)        il_log_real(il_prettifyFile(__FILE__), __LINE__, __func__, 3, __VA_ARGS__)
/** Functionality that is not being enabled, otherwise non-destructive issues the user should know */
#define il_warning(...)    il_log_real(il_prettifyFile(__FILE__), __LINE__, __func__, 2, __VA_ARGS__)
/** A function failed to execute, and returned an errorneous value */
#define il_error(...)      il_log_real(il_prettifyFile(__FILE__), __LINE__, __func__, 1, __VA_ARGS__)
/** The engine can no longer continue functioning and is going to shut down */
#define il_fatal(...) \
    do { \
        il_log_real(il_prettifyFile(__FILE__), __LINE__, __func__, 0, __VA_ARGS__); \
        abort(); \
    } while(0)

/** A function to tell whether a given file can log at the specified level */
int il_can_log(const char *file, int level);

/** Sets the logging level for the specified file */
void il_log_toggle(const char *module, int level);

#endif

