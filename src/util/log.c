#include "log.h"

#include <stdarg.h>

#include "util/uthash.h"
#include "util/logger.h"

char *strdup(const char*);

void il_log_real(const char *file, int line, const char *func, unsigned level, const char *fmt, ...)
{
    il_logger *logger = il_logger_stderr; // TODO: dynamically scoped variable for current logger
    if (il_logger_getFilter(logger) < level) {
        return;
    }
    char* pos = strstr(file, "src/");
    if (pos) {
        file = pos + 4;
    }
    il_logmsg *msg = il_logmsg_new(1);
    il_logmsg_setLevel(msg, level);
    va_list va1, va2;
    va_start(va1, fmt);
    va_copy(va2, va1);
    size_t len = vsnprintf(NULL, 0, fmt, va1);
    va_end(va1);
    char *buf = malloc(len+1);
    vsnprintf(buf, len+1, fmt, va2);
    va_end(va2);
    il_logmsg_setMessage(msg, buf);
    il_logmsg_setBtFile(msg, 0, file, line, func);
    il_logger_log(logger, msg);
}
