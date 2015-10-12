#ifndef ILG_CONTEXT_H
#define ILG_CONTEXT_H

#include <stdlib.h>
#include <SDL_video.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdbool.h>

#include "tgl/tgl.h"
#include "util/array.h"
#include "graphics/renderer.h"

enum ilG_context_attachments {
    ILG_CONTEXT_ALBEDO,     // Ratio of light reflected in RGB
    ILG_CONTEXT_NORMAL,     // Surface normal in camera-local space (world space centered on camera position)
    ILG_CONTEXT_REFRACTION, // Refractive index
    ILG_CONTEXT_GLOSS,      // How shiny or glossy the surface should be
    ILG_CONTEXT_EMISSION,   // Emission from the material itself, measured in radiant intensity (W/sr)
    ILG_CONTEXT_DEPTH,
    ILG_CONTEXT_NUMATTACHMENTS
};

// Accumulation buffer is measured in irradiance (W/m^2)

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
    /** Enable Multisampled Anti-aliasing.
     * Defaults to 0 (disabled). Value indicates number of samples, should be power of four */
    ILG_CONTEXT_MSAA,
    /** Enable sRGB colour space for default framebuffer.
     * Defaults to 1 (enabled). */
    ILG_CONTEXT_SRGB,
};

struct ilG_fbo;
struct ilG_context;

/** Contains state related to OpenGL contexts and window management
 * This structure is the entire interface to and most of the state of the corresponding graphics thread. Graphics is done on a separate thread to separate logic, IO, etc. from rendering. There are multiple sections.
 *
 * - Public members, which can be accessed at any time.
 * - Semi-public members, which should only be accessed by functions run on the rendering thread.
 * - Members used for hints at thread startup (window title, GL version). */
typedef struct ilG_context {
    /* Public members */
    int width, height;
    char *title;
    float fovsquared; // Field of view in radians squared, needed by parts of renderer
    /* For rendering */
    ilG_renderman manager;
    /* Private */
    bool valid;
    tgl_fbo gbuffer, accum;
    int tick_id;
    size_t num_active;
    SDL_Window *window;
    SDL_GLContext context;
    pthread_t thread;
    bool running, complete, have_khr_debug;
    /* Creation parameters */
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
    bool srgb;
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
/** Destroys the context and stops the render thread. Blocks. */
void ilG_context_end(ilG_context *self);

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
bool /*success*/ ilG_context_setupSDLWindow(ilG_context *context);
void ilG_context_setupGLEW(ilG_context *context);
void ilG_context_setupEpoxy(ilG_context *context);
void ilG_context_localSetup(ilG_context *context);
void ilG_context_measure(ilG_context *context);
bool ilG_context_build(void *obj, ilG_rendid id, ilG_renderman *renderman, ilG_buildresult *out);

void ilG_context_print(ilG_context *self);

#endif
