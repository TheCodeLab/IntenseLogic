#include "log.h"

#include <stdarg.h>

#include "util/uthash.h"
#include "util/logger.h"

void il_log_real(const char *file, int line, const char *func, unsigned level, const char *fmt, ...)
{
    il_logger *logger = &il_logger_stderr; // TODO: dynamically scoped variable for current logger
    if (logger->filter < level) {
        return;
    }
    char* pos = strstr(file, "src/");
    if (pos) {
        file = pos + 4;
    }
    va_list va1, va2;
    va_start(va1, fmt);
    va_copy(va2, va1);
    size_t msg_len = vsnprintf(NULL, 0, fmt, va1);
    va_end(va1);
    char *msg_str = malloc(msg_len+1);
    vsnprintf(msg_str, msg_len+1, fmt, va2);
    va_end(va2);

    il_logmsg msg;
    memset(&msg, 0, sizeof(il_logmsg));
    msg.msg = il_string_bin(msg_str, msg_len);
    msg.file = il_string_new((char*)file);
    msg.func = il_string_new((char*)func);
    msg.line = line;
    msg.level = level;
    il_logger_log(logger, msg);
}
