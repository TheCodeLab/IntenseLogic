#ifndef ILG_CONTEXT_H
#define ILG_CONTEXT_H

#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <sys/time.h>

#include "util/array.h"
#include "util/list.h"
#include "common/storage.h"
#include "graphics/bindable.h"
#include "input/input.h"

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
};

/** A linked list node for keeping track of the current framerate */
struct ilG_frame {
    struct timeval start, elapsed;
    IL_LIST(struct ilG_frame) ll;
};

struct ilG_fbo;
struct ilG_context;

/** Contains state related to OpenGL contexts and window management
 * A large structure which is the top-level when it comes to rendering. It controls the framerate, context management, window management, and is where you go to configure the rendering pipeline, resize the window, etc. */
typedef struct ilG_context { // **remember to update context.lua**
    /* Public members */
    il_table storage;
    int width, height;
    struct ilG_frame frames_head;
    struct timeval frames_sum,
                   frames_average;
    size_t num_frames;
    char *title;
    struct ilG_camera* camera;
    struct il_world* world;
    ilE_handler *tick,
                *resize,
                *close,
                *destroy;
    ilI_handler handler;
    /* For rendering */
    struct il_positionable *positionable;
    struct ilG_drawable3d* drawable;
    struct ilG_material* material;
    struct ilG_texture* texture;
    const struct ilG_bindable *drawableb, *materialb, *textureb;
    /* Private */
    int valid;
    GLFWwindow *window;
    GLuint fbtextures[ILG_CONTEXT_NUMATTACHMENTS], framebuffer;
    IL_ARRAY(struct ilG_stage,) stages;
    int tick_id;
    size_t num_active;
    unsigned *texunits;
    size_t num_texunits;
    /* Creation parameters */
    int complete;
    int contextMajor;
    int contextMinor;
    int forwardCompat;
    enum ilG_context_profile profile;
    int debug_context;
    int experimental;
    int startWidth;
    int startHeight;
    int hdr;
    int use_default_fb;
    int debug_render;
    char *initialTitle;
} ilG_context;

ilG_context *ilG_context_new();
/** Destroys the window, associated GL context, and all owned memory */
void ilG_context_free(ilG_context *self);

/** Sets a hint on a context for how it should be constructed. */
void ilG_context_hint(ilG_context *self, enum ilG_context_hint hint, int param);
/** Actually creates the window and GL context. 
 * @return Success. */
int ilG_context_build(ilG_context *self);
/** Resizes (and creates if first call) the context's framebuffers and calls the #ilG_context.resize event. 
 * @return Success. */
int ilG_context_resize(ilG_context *self, int w, int h, const char *title);
/** Start rendering.
 * @return Success. */
int ilG_context_start(ilG_context* self);
/** Inserts a rendering stage into the rendering pipeline. */
void ilG_context_addStage(ilG_context* self, struct ilG_stage stage, int num);
/** Clears the rendering pipeline */
void ilG_context_clearStages(ilG_context *self);
/** Internal: Binds the GL context to the current thread. */
void ilG_context_makeCurrent(ilG_context *self);
/** Internal: Binds the context's internal framebuffer */
void ilG_context_bindFB(ilG_context *self);
/** Internal: Special case function which will be around until #ilG_context is changed to use #ilG_fbo */
void ilG_context_bind_for_outpass(ilG_context *self);

#endif

