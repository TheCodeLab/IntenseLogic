#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "util/array.h"
#include "util/event.h"

il_logger *il_logger_new(const char *name)
{
    il_logger *self = malloc(sizeof(il_logger));
    il_logger_init(self, name);
    return self;
}

void il_logger_init(il_logger *self, const char *name)
{
    memset(self, 0, sizeof(il_logger));
    if (!name) {
        name = "Unknown";
    }
    strcpy(self->name, name);
    self->filter = IL_NOTIFY;
    char hname[strlen(name) + 64];
    sprintf(hname, "Message Handler for Logger %s", name);
    ilE_handler_init_with_name(&self->handler, hname);
}

void il_logger_destroy(il_logger *self)
{
    ilE_handler_destroy(&self->handler);
}

void il_logger_forward(il_logger *from, il_logger *to, enum il_loglevel filter)
{
    il_logforward fwd = (il_logforward){to, filter};
    IL_APPEND(from->forwards, fwd);
}

bool il_logger_log(il_logger *self, il_logmsg msg)
{
    if (!il_logmsg_isLevel(&msg, self->filter)) {
        return false;
    }
    if (self->func) {
        self->func(&msg);
    }
    il_storage_void sv = {&msg, NULL};
    il_value v = il_value_opaque(sv);
    ilE_handler_fire(&self->handler, &v);
    size_t i;
    for (i = 0; i < self->forwards.length; i++) {
        if (il_logmsg_isLevel(&msg, self->forwards.data[i].filter)) {
            il_logger_log(self->forwards.data[i].logger, msg);
        }
    }
    return true;
}

bool il_logmsg_isLevel(const il_logmsg *self, enum il_loglevel level)
{
    return self->level <= level;
}

static void log_stderr(il_logmsg *msg)
{
    fputc('(', stderr);
    if (msg->file.len) {
        fprintf(stderr, "%s:%i", msg->file.str, msg->line);
        if (msg->func.len) {
            fputc(' ', stderr);
        }
    }
    if (msg->func.len) {
        fputs(msg->func.str, stderr);
    }
    fputs(") ", stderr);
    if (msg->msg.len) {
        fputs(msg->msg.str, stderr);
        if (msg->reason.len) {
            fputs(": ", stderr);
        }
    }
    if (msg->reason.len) {
        fputs(msg->reason.str, stderr);
    }
    fputc('\n', stderr);
    if (msg->extra.len) {
        fputs(msg->extra.str, stderr);
    }
}

il_logger il_logger_stderr = {
    .func = log_stderr,
    .filter = IL_NOTIFY,
    .name = "stderr"
};
