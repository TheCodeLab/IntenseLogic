#ifndef COMMON_EVENT_H
#define COMMON_EVENT_H

#include <stdint.h>
#include <stddef.h>
#include <sys/time.h>

struct il_type;
struct il_base;

typedef struct ilE_registry ilE_registry;

enum ilE_behaviour {
    ILE_DONTCARE,
    ILE_BEFORE,
    ILE_AFTER,
    ILE_OVERRIDE
};

enum ilE_threading {
    ILE_ANY,
    ILE_MAIN,
    ILE_TLS,
};

typedef void(*ilE_callback)(const ilE_registry* registry, const char *name, size_t size, const void *data, void * ctx);

ilE_registry* ilE_registry_new();

void ilE_registry_forward(ilE_registry *from, ilE_registry *to);

void ilE_globalevent(   ilE_registry* registry, const char *name, size_t size, const void *data);
void ilE_typeevent  (   struct il_type* type,   const char *name, size_t size, const void *data);
void ilE_objectevent(   struct il_base* base,   const char *name, size_t size, const void *data);

void ilE_globaltimer(   ilE_registry* registry, const char *name, size_t size, const void *data, struct timeval tv);
void ilE_typetimer  (   struct il_type* type,   const char *name, size_t size, const void *data, struct timeval tv);
void ilE_objecttimer(   struct il_base* base,   const char *name, size_t size, const void *data, struct timeval tv);

int ilE_register(ilE_registry* registry, const char *name, enum ilE_behaviour behaviour, enum ilE_threading threads, ilE_callback callback, void * ctx);

void ilE_dumpHooks(ilE_registry *registry);

extern ilE_registry* il_registry;

#endif

