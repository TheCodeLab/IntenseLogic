#include "log.h"

#include <stdarg.h>

const char* il_Common_loglevel_str[] = {"FATAL", "ERROR", "WARNING", "NOTICE", "DEBUG", "FLOOD"};

void il_Common_log_(const char* file, int line, const char *func, int level,
                    const char *fmt, ...)
{
    if (level > (int)il_Common_loglevel) return;

    fprintf(il_Common_logfile, "%s:%i (%s) %s: ",
            il_Common_prettifyFile(file),
            line,
            func,
            il_Common_loglevel_tostring(level)
           );

    va_list ap;
    va_start(ap, fmt);
    vfprintf(il_Common_logfile, fmt, ap);
    va_end(ap);

    fputc('\n', il_Common_logfile);
}
