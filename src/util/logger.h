#ifndef ILU_LOGGER_H
#define ILU_LOGGER_H

#include <stdbool.h>

#include "util/ilstring.h"
#include "util/event.h"
#include "util/array.h"

enum il_loglevel {
    IL_ERROR,
    IL_WARNING,
    IL_NOTIFY,
    IL_DEBUG
};

typedef struct il_logmsg {
    il_string msg, reason, file, func, extra;
    int line;
    enum il_loglevel level;
} il_logmsg;

typedef struct il_logforward {
    struct il_logger *logger;
    enum il_loglevel filter;
} il_logforward;

typedef struct il_logger {
    void (*func)(il_logmsg *msg);
    ilE_handler handler;
    enum il_loglevel filter;
    IL_ARRAY(il_logforward,) forwards;
    char name[64];
} il_logger;

il_logger *il_logger_new(const char *name);
void il_logger_init(il_logger *self, const char *name);
void il_logger_destroy(il_logger *self);
il_logger *il_logger_cur();
void il_logger_push(il_logger *self);
void il_logger_pop();

void il_logger_forward(il_logger *from, il_logger *to, enum il_loglevel filter);
bool /*success*/ il_logger_log(il_logger *self, il_logmsg msg);
bool il_logmsg_isLevel(const il_logmsg *self, enum il_loglevel level);

extern il_logger il_logger_stderr;

#endif
