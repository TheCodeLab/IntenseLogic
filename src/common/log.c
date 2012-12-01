#include "log.h"

#include <stdarg.h>

const char* il_loglevel_str[] = {"FATAL", "ERROR", "WARNING", "NOTICE", "DEBUG", "FLOOD"};

void il_log_(const char* file, int line, const char *func, int level,
                    const char *fmt, ...)
{
    if (level > (int)il_loglevel) return;

    fprintf(il_logfile, "%s:%i (%s) %s: ",
            il_prettifyFile(file),
            line,
            func,
            il_loglevel_tostring(level)
           );

    va_list ap;
    va_start(ap, fmt);
    vfprintf(il_logfile, fmt, ap);
    va_end(ap);

    fputc('\n', il_logfile);
}
