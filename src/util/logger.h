#ifndef ILU_LOGGER_H
#define ILU_LOGGER_H

typedef struct il_logger il_logger;

enum il_loglevel {
    IL_ERROR,
    IL_WARNING,
    IL_NOTIFY,
    IL_DEBUG
};

enum il_logtype {
    IL_LOGTYPE_STRING,
    IL_LOGTYPE_FILE,
};

typedef struct il_logmsg il_logmsg;
typedef struct il_logsource il_logsource;

typedef struct il_logtype_file {
    int line;
    char name[64];
    char func[64];
} il_logtype_file;

il_logger *il_logger_new(const char *name);
void il_logger_destroy(il_logger *self);

struct ilE_handler;
struct ilE_handler *il_logger_getHandler(il_logger *self);
enum il_loglevel il_logger_getFilter(const il_logger *self);
enum il_loglevel il_logger_getBacktraceFilter(const il_logger *self);
int il_logger_getMessageStart(const il_logger *self);
int il_logger_getMessageEnd(const il_logger *self);
const il_logmsg *il_logger_getMessage(const il_logger *self, int id);

void il_logger_setFilter(il_logger *self, enum il_loglevel level);
void il_logger_setBacktraceFilter(il_logger *self, enum il_loglevel level);
void il_logger_setBuffering(il_logger *self, int messages);

void il_logger_forward(il_logger *from, il_logger *to, enum il_loglevel filter);
// consumes msg
void il_logger_log(il_logger *self, il_logmsg *msg);

il_logmsg *il_logmsg_new(unsigned btsize);
il_logmsg *il_logmsg_copy(il_logmsg *self);
void il_logmsg_destroy(il_logmsg *self);

enum il_loglevel il_logmsg_getLevel(const il_logmsg *self);
const char *il_logmsg_getMessage(const il_logmsg *self);
enum il_logtype il_logmsg_getBtType(const il_logmsg *self, unsigned level);
const char *il_logmsg_getBtSource(const il_logmsg *self, unsigned level);
const il_logtype_file *il_logmsg_getBtFile(const il_logmsg *self, unsigned level);

void il_logmsg_setLevel(il_logmsg *self, enum il_loglevel level);
// consumes msg
void il_logmsg_setMessage(il_logmsg *self, char *msg);
void il_logmsg_copyMessage(il_logmsg *self, const char *msg);
// consumes msg
void il_logmsg_setReason(il_logmsg *self, char *msg);
void il_logmsg_copyReason(il_logmsg *self, const char *msg);
// consumes source
void il_logmsg_setBtString(il_logmsg *self, unsigned level, char *source);
void il_logmsg_copyBtString(il_logmsg *self, unsigned leve, const char *source);
void il_logmsg_setBtFile(il_logmsg *self, unsigned level, const char *name, int line, const char *func);

int il_logmsg_isLevel(const il_logmsg *self, enum il_loglevel level);

extern il_logger *il_logger_stderr;

#endif

