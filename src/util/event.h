/** @file event.h
 * @brief Event system
 *
 * The event mechanism is how many things are propagated with IL, they can be fired from anywhere, and anyone can hook them. The "main loop" is really just calling into libevent and waiting on events to process.
 *
 * @section Overview
 * Every handler contains a list of callbacks, which can be added and removed
 * with ilE_register() and ilE_unregister, respectively. This means there is
 * a 1:N relationship between handlers and callbacks, which allows multiple
 * parts of the codebase to listen to the same events without running into
 * each other.
 *
 * @section Example
 * @code
 * ilE_handler *handler = ilE_handler_timer(1); // Create a 1-second timer
 * int cb1 = ilE_register(handler, ILE_DONTCARE, ILE_ANY, &some_cb, NULL); // Add a callback
 * int cb2 = ilE_register(handler, ILE_AFTER, ILE_ANY, &some_other_cb, NULL) // Add a callback which runs after cb1
 * ilE_unregister(cb1); // Remove cb1, leaving just cb2
 * ilE_unregister(cb2); // Remove cb2, leaving a no-op event handler
 * ilE_handler_destroy(handler); // Destory the event handler
 * @endcode
 */

#ifndef COMMON_EVENT_H
#define COMMON_EVENT_H

#include <stdint.h>
#include <stddef.h>
#include <sys/time.h>

#include "common/storage.h"

typedef struct ilE_handler ilE_handler;

/*! The behaviour when registering an event hook, on how important it is
 * @see ilE_register */
enum ilE_behaviour {
    ILE_DONTCARE,       /**< No preference for when the event is called */
    ILE_BEFORE = -100,  /**< Must be called before previously registered handlers */
    ILE_AFTER = 100,    /**< Must be called after previously registered handlers */
};

/*! How the handler should schedule the callback
 * @see ilE_register */
enum ilE_threading {
    ILE_ANY,            /**< Runs on any of the worker threads */
    ILE_MAIN,           /**< Runs only on the main thread */
    ILE_TLS,
};

/*! Callback type for events */
typedef void(*ilE_callback)(const il_value *data, il_value *ctx);

/*! Creates a normal handler which is only fired by the user. */
ilE_handler *ilE_handler_new();
/*! Combination of ilE_handler_new() and ilE_handler_name(). */
ilE_handler *ilE_handler_new_with_name(const char *name);
/*! Destroy a handler. Make sure all callbacks have been cleaned up first! */
void ilE_handler_destroy(ilE_handler *self);

/*! Gets the current name of the handler. Data lasts for lifetime of handler. */
const char *ilE_handler_getName(const ilE_handler *self);

/*! Set a name. Used for introspective purposes such as debugging. */
void ilE_handler_name(ilE_handler *self, const char *name);

/*! Fires an event for a handler immediately, using no copies. */
void ilE_handler_fire(ilE_handler *self, const il_value *data);

/*! Registers a handler for a given event.
 * @param name A string name used for introspection, such as debugging
 * @return A unique handle into the handler which can be used to unregister the callback.
 * @see ilE_unregister */
int ilE_register_real(ilE_handler* self, const char *name, int priority, enum ilE_threading threads, ilE_callback callback, il_value ctx);
/*! Convienience wrapper which sets the name to __func__ */
#define ilE_register(self, b, t, cb, ctx) ilE_register_real(self, #cb, b, t, cb, ctx)
/*! Deletes a callback - use this on all your registered callbacks before destroying a handler. */
void ilE_unregister(ilE_handler *self, int handle);

/*! Dumps a list of all the callbacks currently set to stderr.
 * Uses the naming information provided by the other functions. */
void ilE_dump(ilE_handler *self);

/*! Called to shutdown the engine. */
extern ilE_handler *ilE_shutdown;
/*! Called to destroy all callbacks currently registered, except for those registered to this one and the shutdownHandlers event. */
extern ilE_handler *ilE_shutdownCallbacks;
/*! Called to destroy all handlers except for this one. */
extern ilE_handler *ilE_shutdownHandlers;

#endif
