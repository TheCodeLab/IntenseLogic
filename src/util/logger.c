#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct logger_node {
    il_logger *logger;
    struct logger_node *next;
} logger_node;

#ifdef _WIN32

#include <windows.h>
static CRITICAL_SECTION il_logger_mutex;
static bool il_logger_created = false;
__declspec(thread) logger_node *il_logger_head;

static void il_logger_lock()
{
    if (!il_logger_created) {
        il_logger_created = true;
        InitializeCriticalSection(&il_logger_mutex);
    }
    EnterCriticalSection(&il_logger_mutex);
}

static void il_logger_unlock()
{
    LeaveCriticalSection(&il_logger_mutex);
}

#else

#include <pthread.h>
static pthread_mutex_t il_logger_mutex;
static bool il_logger_created = false;
static __thread logger_node *il_logger_head;

static void il_logger_lock()
{
    if (!il_logger_created) {
        il_logger_created = true;
        pthread_mutex_init(&il_logger_mutex, NULL);
    }
    pthread_mutex_lock(&il_logger_mutex);
}

static void il_logger_unlock()
{
    pthread_mutex_unlock(&il_logger_mutex);
}

#endif

#include "util/array.h"

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
}

void il_logger_destroy(il_logger *self)
{
    (void)self;
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
    il_logger_lock();
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
    il_logger_unlock();
}

il_logger il_logger_stderr = {
    .func = log_stderr,
    .filter = IL_NOTIFY,
    .name = "stderr"
};

il_logger *il_logger_cur()
{
    if (!il_logger_head || !il_logger_head->logger) {
        return &il_logger_stderr;
    }
    return il_logger_head->logger;
}

void il_logger_push(il_logger *self)
{
    logger_node *node = calloc(1, sizeof(logger_node));
    node->logger = self;
    if (il_logger_head) {
        node->next = il_logger_head;
    }
    il_logger_head = node;
}

void il_logger_pop()
{
    logger_node *node = il_logger_head;
    assert(node && "No loggers left to pop");
    il_logger_head = il_logger_head->next;
    free(node);
}
