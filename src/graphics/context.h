#ifndef ILG_CONTEXT_H
#define ILG_CONTEXT_H

#include <stdlib.h>
#include <GL/glew.h>
#include <SDL2/SDL_video.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdbool.h>

#include "util/array.h"
#include "util/list.h"
#include "common/storage.h"
#include "input/input.h"
#include "graphics/renderer.h"
#include "tgl/tgl.h"

struct ilG_stage;

enum ilG_context_attachments {
    ILG_CONTEXT_DEPTH,
    ILG_CONTEXT_ACCUM,
    ILG_CONTEXT_NORMAL,
    ILG_CONTEXT_DIFFUSE,
    ILG_CONTEXT_SPECULAR,
    ILG_CONTEXT_NUMATTACHMENTS
};

enum ilG_context_profile {
    ILG_CONTEXT_NONE,
    ILG_CONTEXT_CORE,
    ILG_CONTEXT_COMPAT
};

/** Hint names for ilG_context_hint() */
enum ilG_context_hint {
    /** OpenGL context major (the number before the dot) version.
     * Defaults to 3. */
    ILG_CONTEXT_MAJOR,
    /** OpenGL context minor (the number after the dot) version.
     * Defaults to 1 (2 on OS X). */
    ILG_CONTEXT_MINOR,
    /** Sets the context as being forward compatible with newer functionality.
     * Defaults to `GL_FALSE`. */
    ILG_CONTEXT_FORWARD_COMPAT,
    /** Sets which profile to use for the OpenGL context.
     * Defaults to #ILG_CONTEXT_CORE.
     * @see ilG_context_profile */
    ILG_CONTEXT_PROFILE,
    /** Sets the context as being a debug context.
     * Defaults to `GL_TRUE`. Required for `KHR_debug` on some implementations (namely, Nvidia). */
    ILG_CONTEXT_DEBUG_CONTEXT,
    /** Set the `glfwExperimental` flag.
     * Defaults to `GL_TRUE`. Last it was tested, the engine would crash without this flag set. */
    ILG_CONTEXT_EXPERIMENTAL,
    /** Window width.
     * Defaults to 800. */
    ILG_CONTEXT_WIDTH,
    /** Window height.
     * Defaults to 600. */
    ILG_CONTEXT_HEIGHT,
    /** Use HDR rendering.
     * Defaults to 0. */
    ILG_CONTEXT_HDR,
    /** Use the default framebuffer, disabling post-processing.
     * Defaults to 0. */
    ILG_CONTEXT_USE_DEFAULT_FB,
    /** Enable some cautious debug rendering functionality.
     * Defaults to 0. Causes performance warnings on some systems. */
    ILG_CONTEXT_DEBUG_RENDER,
    /** Enable vsync */
    ILG_CONTEXT_VSYNC,
    /** Enable Multisampled Anti-aliasing */
    ILG_CONTEXT_MSAA,
};

/** A linked list node for keeping track of the current framerate */
struct ilG_frame {
    struct timeval start, elapsed;
    IL_LIST(struct ilG_frame) ll;
};

struct ilG_fbo;
struct ilG_context;

/** Contains state related to OpenGL contexts and window management
 * This structure is the entire interface to and most of the state of the corresponding graphics thread. Graphics is done on a separate thread to separate logic, IO, etc. from rendering. There are multiple sections.
 *
 * - Public members, which can be accessed at any time.
 * - Semi-public members, which should only be accessed by functions run on the rendering thread.
 * - Members used for hints at thread startup (window title, GL version). */
typedef struct ilG_context { // **remember to update context.lua**
    /* Public members */
    il_table storage;
    int width, height;
    struct ilG_frame frames_head;
    struct timeval frames_sum,
                   frames_average;
    size_t num_frames;
    char *title;
    ilE_handler *tick,
                *resize,
                *close,
                *destroy;
    ilI_handler handler;
    ilG_handle root;
    /* For rendering */
    ilG_rendermanager manager;
    /* Legacy rendering */
    unsigned *texunits;
    size_t num_texunits;
    /* Private */
    bool valid;
    tgl_fbo fb;
    int tick_id;
    size_t num_active;
    struct ilG_context_queue *queue;
    SDL_Window *window;
    SDL_GLContext context;
    pthread_t thread;
    bool running;
    /* Creation parameters */
    bool complete;
    int contextMajor;
    int contextMinor;
    bool forwardCompat;
    enum ilG_context_profile profile;
    bool debug_context;
    bool experimental;
    int startWidth;
    int startHeight;
    bool hdr;
    bool use_default_fb;
    bool debug_render;
    char *initialTitle;
    bool vsync;
    bool msaa;
} ilG_context;

ilG_context *ilG_context_new();
void ilG_context_init(ilG_context *context);
/** Destroys the window, associated GL context, and all owned memory */
void ilG_context_free(ilG_context *self);

/* Pre-start functions */
/** Sets a hint on a context for how it should be constructed. */
void ilG_context_hint(ilG_context *self, enum ilG_context_hint hint, int param);
/** Start rendering.
 * @return Success. */
bool ilG_context_start(ilG_context* self);
/** Stops the render thread. Blocks. */
void ilG_context_stop(ilG_context *self);

/* External calls */
/** Calls a function at the beginning of the frame on the context thread, usually for building VBOs */
bool ilG_context_upload(ilG_context *self, void (*fn)(void*), void*);
/** Resizes (and creates if first call) the context's framebuffers and calls the #ilG_context.resize event.
 * @return Success. */
bool ilG_context_resize(ilG_context *self, int w, int h);
/** Renames the window */
bool ilG_context_rename(ilG_context *self, const char *title);

/* Rendering thread calls */
ilG_renderer    *ilG_context_findRenderer       (ilG_context *self, ilG_rendid id);
ilG_msgsink     *ilG_context_findSink           (ilG_context *self, ilG_rendid id);
il_table        *ilG_context_findStorage        (ilG_context *self, ilG_rendid id);
const char      *ilG_context_findName           (ilG_context *self, ilG_rendid id);
unsigned ilG_context_addRenderer    (ilG_context *self, ilG_rendid id, ilG_builder builder);
unsigned ilG_context_addSink        (ilG_context *self, ilG_rendid id, ilG_message_fn sink);
unsigned ilG_context_addChild       (ilG_context *self, ilG_rendid parent, ilG_rendid child);
unsigned ilG_context_addCoords      (ilG_context *self, ilG_rendid id, ilG_cosysid cosys, unsigned codata);
unsigned ilG_context_viewCoords     (ilG_context *self, ilG_rendid id, ilG_cosysid cosys);
unsigned ilG_context_addLight       (ilG_context *self, ilG_rendid id, struct ilG_light light);
unsigned ilG_context_addStorage     (ilG_context *self, ilG_rendid id);
unsigned ilG_context_addName        (ilG_context *self, ilG_rendid id, const char *name);
unsigned ilG_context_addCoordSys    (ilG_context *self, ilG_coordsys co);
bool ilG_context_delRenderer    (ilG_context *self, ilG_rendid id);
bool ilG_context_delSink        (ilG_context *self, ilG_rendid id);
bool ilG_context_delChild       (ilG_context *self, ilG_rendid parent, ilG_rendid child);
bool ilG_context_delCoords      (ilG_context *self, ilG_rendid id, ilG_cosysid cosys, unsigned codata);
bool ilG_context_delLight       (ilG_context *self, ilG_rendid id, struct ilG_light light);
bool ilG_context_delStorage     (ilG_context *self, ilG_rendid id);
bool ilG_context_delName        (ilG_context *self, ilG_rendid id);
bool ilG_context_delCoordSys    (ilG_context *self, unsigned id);
/** Internal: Binds the context's internal framebuffer */
void ilG_context_bindFB(ilG_context *self);
/** Internal: Special case function which will be around until #ilG_context is changed to use #ilG_fbo */
void ilG_context_bind_for_outpass(ilG_context *self);
/** Render thread call for resizing framebuffer */
int ilG_context_localResize(ilG_context *self, int w, int h);
/** Render for one frame. Useful if you want your own render loop. */
void ilG_context_renderFrame(ilG_context *context);
/** Run the render loop. Useful if you want to pick the thread yourself.
    Takes a void * to fit the pthread function call syntax. It is an ilG_context.

    Steps to running your own render thread:

    - Create the window on the main thread. ilG_context_setupSDLWindow is provided.
    - Create the thread. Pthread is used for this.
    - Setup your GL context. SDL is used for this.
    - Setup GLEW. ilG_context_setupGLEW is provided.
    - Setup the context's render thread stuff using ilG_context_localSetup().
    - Run the render thread. The render thread has several duties.
      + Fire the tick event.
      + Check the message queue. (See context-internal.h for the interface.)
      + Check if the window has resized and call ilG_context_localResize.
      + Render a frame using ilG_context_renderFrame.
      + Swap the window buffer using SDL_GL_SwapWindow.
      + Collect fps statistics with ilG_context_measure.

    ilG_context_start will get everything going by default. You should use it unless you wish to create the thread yourself, or do rendering on the main thread for some reason.

    ilG_context_loop, should you want to create the thread yourself, can be passed directly to pthread_create as such:

        pthread_create(&context->thread, NULL, ilG_context_loop, context);
*/
void *ilG_context_loop(void *context);
void ilG_context_setupSDLWindow(ilG_context *context);
void ilG_context_setupGLEW(ilG_context *context);
void ilG_context_localSetup(ilG_context *context);
void ilG_context_measure(ilG_context *context);
bool ilG_context_build(void *obj, ilG_rendid id, ilG_context *context, ilG_buildresult *out);

void ilG_context_printScenegraph(ilG_context *self);

#endif
