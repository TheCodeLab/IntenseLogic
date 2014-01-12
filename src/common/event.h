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

typedef struct ilE_handler ilE_handler;

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

typedef void(*ilE_callback)(const ilE_handler* registry, size_t size, const void *data, void * ctx);

/** Creates a new handler */
ilE_handler *ilE_handler_new();
ilE_handler *ilE_handler_new_with_name(const char *name);
ilE_handler *ilE_handler_timer(const struct timeval *tv);
ilE_handler *ilE_handler_watch(int fd, enum ilE_fdevent what);
void ilE_handler_destroy(ilE_handler *self);

void ilE_handler_name(ilE_handler *self, const char *name);

/** Fires an event for a handler*/
void ilE_handler_fire(ilE_handler *self, size_t size, const void *data);
void ilE_handler_fireasync(ilE_handler *self, size_t size, const void *data);

/** Registers a handler for a given event */
int ilE_register_real(ilE_handler* self, const char *name, enum ilE_behaviour behaviour, enum ilE_threading threads, ilE_callback callback, void * ctx);
#define ilE_register(self, b, t, cb, ctx) ilE_register_real(self, __func__, b, t, cb, ctx)
/** Unregisters a handler */
void ilE_unregister(ilE_handler *self, int handle);

/** Prints all of the currently registered handlers for the specified registry */
void ilE_dump(ilE_handler *self);

extern ilE_handler *ilE_shutdown;
extern ilE_handler *ilE_shutdownCallbacks;
extern ilE_handler *ilE_shutdownHandlers;

#endif

