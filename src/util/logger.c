#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "util/array.h"
#define IL_DLSYM_LOADER
#include "loader.h"

#define new_args(f) f(const char*, name)
il_gen_symbol("ilcommon", NULL, struct ilE_handler*, ilE_handler_new_with_name, new_args)
#define des_args(f) f(struct ilE_handler*, self)
il_gen_noret_symbol("ilcommon", ilE_handler_destroy, des_args)
#define fire_args(f) f(struct ilE_handler*, self), f(size_t, size), f(const void *, data)
il_gen_noret_symbol("ilcommon", ilE_handler_fire, fire_args)

struct forward {
    il_logger *logger;
    enum il_loglevel filter;
};

struct il_logger {
    il_logmsg **buffer;
    size_t start, end, offset, len, capacity;
    struct ilE_handler *handler;
    enum il_loglevel filter;
    enum il_loglevel btfilter;
    IL_ARRAY(struct forward,) forwards;
    char name[64];
};

struct il_logsource {
    enum il_logtype type;
    union {
        char *source;
        il_logtype_file *file;
    } source;
};

struct il_logmsg {
    enum il_loglevel level;
    char *msg, *reason;
    size_t num_bt;
    il_logsource backtrace[];
};

il_logger *il_logger_new(const char *name)
{
    il_logger *self = calloc(1, sizeof(il_logger));
    strcpy(self->name, name);
    self->filter = IL_NOTIFY;
    self->btfilter = IL_WARNING;
    char hname[strlen(name) + 64];
    sprintf(hname, "Message Handler for Logger %s", name);
    self->handler = ilE_handler_new_with_name(hname);
    return self;
}

void il_logger_destroy(il_logger *self)
{
    ilE_handler_destroy(self->handler);
    free(self->buffer);
    free(self);
}

struct ilE_handler *il_logger_getHandler(il_logger *self)
{
    return self->handler;
}

enum il_loglevel il_logger_getFilter(const il_logger *self)
{
    return self->filter;
}

enum il_loglevel il_logger_getBacktraceFilter(const il_logger *self)
{
    return self->btfilter;
}

int il_logger_getMessageStart(const il_logger *self)
{
    return self->offset;
}

int il_logger_getMessageEnd(const il_logger *self)
{
    return self->offset + self->len - 1;
}

const il_logmsg *il_logger_getMessage(const il_logger *self, int id)
{
    if (id < 0) {
        assert((int)self->len + id >= 0);
        return self->buffer[self->len + id];
    }
    id -= self->offset;
    assert(id > 0 && (size_t)id >= self->len);
    return self->buffer[id];
}

void il_logger_setFilter(il_logger *self, enum il_loglevel level)
{
    self->filter = level;
}

void il_logger_setBacktraceFilter(il_logger *self, enum il_loglevel level)
{
    self->btfilter = level;
}

void il_logger_setBuffering(il_logger *self, int messages)
{
    self->buffer = realloc(self->buffer, sizeof(il_logmsg) * messages);
    self->capacity = messages;
    self->len = 0;
    self->start = 0;
    self->end = messages;
}

void il_logger_forward(il_logger *from, il_logger *to, enum il_loglevel filter)
{
    struct forward fwd = (struct forward){to, filter};
    IL_APPEND(from->forwards, fwd);
}

static void print_source(il_logsource *s)
{
    if (s->type == IL_LOGTYPE_STRING) {
        fprintf(stderr, "%s", s->source.source);
    } else if (s->type == IL_LOGTYPE_FILE) {
        il_logtype_file *f = s->source.file;
        fprintf(stderr, "%s:%i%s%s", f->name, f->line, strlen(f->func)? " " : "", f->func);
    } else {
        fprintf(stderr, "?");
    }
}

static void log_stderr(il_logmsg *msg)
{
    if (msg->num_bt > 0) {
        fputc('(', stderr);
        print_source(msg->backtrace + 0);
        fputs(") ", stderr);
    }
    fprintf(stderr, "%s", msg->msg);
    if (msg->reason) {
        fprintf(stderr, ": %s", msg->reason);
    }
    fputc('\n', stderr);
    if (msg->num_bt > 1 && il_logger_stderr->btfilter >= msg->level) {
        unsigned i;
        for (i = 0; i < msg->num_bt; i++) {
            fprintf(stderr,"\t#%i in ", i);
            print_source(msg->backtrace + i);
            fputc('\n', stderr);
        }
    }
}

void il_logger_log(il_logger *self, il_logmsg *msg)
{
    if (!il_logmsg_isLevel(msg, self->filter)) {
        il_logmsg_destroy(msg);
        return;
    }
    ilE_handler_fire(self->handler, sizeof(il_logmsg), msg);
    size_t i;
    for (i = 0; i < self->forwards.length; i++) {
        if (il_logmsg_isLevel(msg, self->forwards.data[i].filter)) {
            il_logger_log(self->forwards.data[i].logger, il_logmsg_copy(msg));
        }
    }
    if (self->capacity > 0) {
        self->end++;
        self->end %= self->capacity;
        if (self->end == self->start) {
            self->offset++;
            self->start++;
            self->start %= self->capacity;
            il_logmsg_destroy(self->buffer[self->end]);
        } else {
            self->len++;
        }
        self->buffer[self->end] = msg;
    }
    if (self == il_logger_stderr) {
        log_stderr(msg);
    }
}

il_logmsg *il_logmsg_new(unsigned btsize)
{
    il_logmsg *self = calloc(1, sizeof(il_logmsg) + btsize * sizeof(il_logsource));
    self->num_bt = btsize;
    return self;
}

char *strdup(const char*);
il_logmsg *il_logmsg_copy(il_logmsg *self)
{
    il_logmsg *new = il_logmsg_new(self->num_bt);
    new->level = self->level;
    new->msg = strdup(self->msg);
    unsigned i;
    for (i = 0; new->num_bt; i++) {
        new->backtrace[i].type = self->backtrace[i].type;
        switch (new->backtrace[i].type) {
            case IL_LOGTYPE_STRING:
            new->backtrace[i].source.source = strdup(self->backtrace[i].source.source);
            break;
            case IL_LOGTYPE_FILE:
            new->backtrace[i].source.file = calloc(1, sizeof(il_logtype_file));
            memcpy(new->backtrace[i].source.file, self->backtrace[i].source.file, sizeof(il_logtype_file));
            break;
        }
    }
    return new;
}

void il_logmsg_destroy(il_logmsg *self)
{
    free(self->msg);
    unsigned i;
    for (i = 0; i < self->num_bt; i++) {
        // Assumes that char* and il_logtype_file* have the same representation. Deal with it.
        free(self->backtrace[i].source.source);
    }
    free(self);
}

enum il_loglevel il_logmsg_getLevel(const il_logmsg *self)
{
    return self->level;
}

const char *il_logmsg_getMessage(const il_logmsg *self)
{
    return self->msg;
}

enum il_logtype il_logmsg_getBtType(const il_logmsg *self, unsigned level)
{
    assert(level < self->num_bt);
    return self->backtrace[level].type;
}

const char *il_logmsg_getBtSource(const il_logmsg *self, unsigned level)
{
    assert(level < self->num_bt);
    assert(self->backtrace[level].type == IL_LOGTYPE_STRING);
    return self->backtrace[level].source.source;
}

const il_logtype_file *il_logmsg_getBtFile(const il_logmsg *self, unsigned level)
{
    assert(level < self->num_bt);
    assert(self->backtrace[level].type == IL_LOGTYPE_FILE);
    return self->backtrace[level].source.file;
}

void il_logmsg_setLevel(il_logmsg *self, enum il_loglevel level)
{
    self->level = level;
}

void il_logmsg_setMessage(il_logmsg *self, char *msg)
{
    self->msg = msg;
}

void il_logmsg_copyMessage(il_logmsg *self, const char *msg)
{
    self->msg = strdup(msg);
}

void il_logmsg_setReason(il_logmsg *self, char *msg)
{
    self->reason = msg;
}

void il_logmsg_copyReason(il_logmsg *self, const char *msg)
{
    self->reason = strdup(msg);
}

void il_logmsg_setBtString(il_logmsg *self, unsigned level, char *source)
{
    assert(level < self->num_bt);
    self->backtrace[level].type = IL_LOGTYPE_STRING;
    self->backtrace[level].source.source = source;
}

void il_logmsg_copyBtString(il_logmsg *self, unsigned level, const char *source)
{
    assert(level < self->num_bt);
    self->backtrace[level].type = IL_LOGTYPE_STRING;
    self->backtrace[level].source.source = strdup(source);
}

void il_logmsg_setBtFile(il_logmsg *self, unsigned level, const char *name, int line, const char *func)
{
    assert(level < self->num_bt);
    self->backtrace[level].type = IL_LOGTYPE_FILE;
    self->backtrace[level].source.file = calloc(1, sizeof(il_logtype_file));
    self->backtrace[level].source.file->line = line;
    strcpy(self->backtrace[level].source.file->name, name);
    strcpy(self->backtrace[level].source.file->func, func);
}

int il_logmsg_isLevel(const il_logmsg *self, enum il_loglevel level)
{
    return self->level <= level;
}

void il_logger_init()
{
    il_logger_stderr = il_logger_new("stderr");
}

il_logger *il_logger_stderr;

