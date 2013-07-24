/** @file event.h
 * @brief Event handling code
 *
 * The event mechanism is how many things are propagated with IL, they can be fired from anywhere, and anyone can hook them. The "main loop" is really just calling into libevent and waiting on events to process.
 */

#ifndef COMMON_EVENT_H
#define COMMON_EVENT_H

#include <stdint.h>
#include <stddef.h>
#include <sys/time.h>

struct il_type;
struct il_base;

typedef struct ilE_registry ilE_registry;

/** The behaviour when registering an event hook, on how important it is */
enum ilE_behaviour {
    ILE_DONTCARE,   /**< No preference for when the event is called */
    ILE_BEFORE,     /**< Must be called before previously registered handlers */
    ILE_AFTER,      /**< Must be called after previously registered handlers */
    ILE_OVERRIDE,   /**< Only call this handler, ignoring the others */
};

/** How the handler should schedule the callback */
enum ilE_threading {
    ILE_ANY,    /**< Runs on any of the worker threads */
    ILE_MAIN,   /**< Runs only on the main thread */
    ILE_TLS,
};

/** What events to wait for on a file descriptor */
enum ilE_fdevent {
    ILE_READ = 1,       /**< When data is ready to be read, call the handlers */
    ILE_READASARG = 2,  /**< Reads the ready data from the file descriptor and passes it to callbacks; ONLY USE IF O_NONBLOCK IS SET! */
    ILE_WRITE = 4,      /**< When the FD is ready to be written to, call the handlers */
};

typedef void(*ilE_callback)(const ilE_registry* registry, const char *name, size_t size, const void *data, void * ctx);

/** Creates a new registry */
ilE_registry* ilE_registry_new();

/** Sets a registry to forward all of the events it receives to another registry */
void ilE_registry_forward(ilE_registry *from, ilE_registry *to);

/** Fires an event for a registry */
void ilE_globalevent(   ilE_registry* registry, const char *name, size_t size, const void *data);
/** Fires an event to a type */
void ilE_typeevent  (   struct il_type* type,   const char *name, size_t size, const void *data);
/** Fires an event to an object */
void ilE_objectevent(   struct il_base* base,   const char *name, size_t size, const void *data);

/** Starts a timer on a registry */
void ilE_globaltimer(   ilE_registry* registry, const char *name, size_t size, const void *data, struct timeval tv);
/** Starts a timer on a type */
void ilE_typetimer  (   struct il_type* type,   const char *name, size_t size, const void *data, struct timeval tv);
/** Starts a timer on an object */
void ilE_objecttimer(   struct il_base* base,   const char *name, size_t size, const void *data, struct timeval tv);

/** Watches for the given fd events on the specified registry */
void ilE_watchfd(ilE_registry* registry, const char *name, int fd, int what);

/** Registers a handler for a given event */
int ilE_register(ilE_registry* registry, const char *name, enum ilE_behaviour behaviour, enum ilE_threading threads, ilE_callback callback, void * ctx);

/** Prints all of the currently registered handlers for the specified registry */
void ilE_dumpHooks(ilE_registry *registry);

/** Registry for the common module */
extern ilE_registry* il_registry;

#endif

