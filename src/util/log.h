/** @file log.h
 * @brief Responsible for logging facilities and limiting flooding from irrelevant modules while debugging
 */

#ifndef ILU_LOG_H
#define ILU_LOG_H

#include <string.h>
#include <stdio.h>

void il_log_real(const char *file, int line, const char *func, unsigned level, const char *fmt, ...) __attribute__((format(printf, 5, 6)));

/** For information that would only be useful to someone debugging the module itself */
#define il_debug(...)      il_log_real(__FILE__, __LINE__, __func__, 3, __VA_ARGS__)
/** Informative messages */
#define il_log(...)        il_log_real(__FILE__, __LINE__, __func__, 2, __VA_ARGS__)
/** Functionality that is not being enabled, otherwise non-destructive issues the user should know */
#define il_warning(...)    il_log_real(__FILE__, __LINE__, __func__, 1, __VA_ARGS__)
/** A function failed to execute, and returned an errorneous value */
#define il_error(...)      il_log_real(__FILE__, __LINE__, __func__, 0, __VA_ARGS__)

#endif
