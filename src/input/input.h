#ifndef ILI_INPUT_H
#define ILI_INPUT_H

/** Key mapping table */
enum ilI_key {
    // most of these were lifted off of GFLW so they wouldn't have to be arbitrarily assigned and remapped
    ILI_KEY_WORLD_1         = 161, /* non-US #1 */
    ILI_KEY_WORLD_2         = 162, /* non-US #2 */
    ILI_KEY_ESCAPE          = 256,
    ILI_KEY_ENTER           = 257,
    ILI_KEY_TAB             = 258,
    ILI_KEY_BACKSPACE       = 259,
    ILI_KEY_INSERT          = 260,
    ILI_KEY_DELETE          = 261,
    ILI_KEY_RIGHT           = 262,
    ILI_KEY_LEFT            = 263,
    ILI_KEY_DOWN            = 264,
    ILI_KEY_UP              = 265,
    ILI_KEY_PAGE_UP         = 266,
    ILI_KEY_PAGE_DOWN       = 267,
    ILI_KEY_HOME            = 268,
    ILI_KEY_END             = 269,
    ILI_KEY_CAPS_LOCK       = 280,
    ILI_KEY_SCROLL_LOCK     = 281,
    ILI_KEY_NUM_LOCK        = 282,
    ILI_KEY_PRINT_SCREEN    = 283,
    ILI_KEY_PAUSE           = 284,
    ILI_KEY_F1              = 290,
    ILI_KEY_F2              = 291,
    ILI_KEY_F3              = 292,
    ILI_KEY_F4              = 293,
    ILI_KEY_F5              = 294,
    ILI_KEY_F6              = 295,
    ILI_KEY_F7              = 296,
    ILI_KEY_F8              = 297,
    ILI_KEY_F9              = 298,
    ILI_KEY_F10             = 299,
    ILI_KEY_F11             = 300,
    ILI_KEY_F12             = 301,
    ILI_KEY_F13             = 302,
    ILI_KEY_F14             = 303,
    ILI_KEY_F15             = 304,
    ILI_KEY_F16             = 305,
    ILI_KEY_F17             = 306,
    ILI_KEY_F18             = 307,
    ILI_KEY_F19             = 308,
    ILI_KEY_F20             = 309,
    ILI_KEY_F21             = 310,
    ILI_KEY_F22             = 311,
    ILI_KEY_F23             = 312,
    ILI_KEY_F24             = 313,
    ILI_KEY_F25             = 314,
    ILI_KEY_KP_0            = 320,
    ILI_KEY_KP_1            = 321,
    ILI_KEY_KP_2            = 322,
    ILI_KEY_KP_3            = 323,
    ILI_KEY_KP_4            = 324,
    ILI_KEY_KP_5            = 325,
    ILI_KEY_KP_6            = 326,
    ILI_KEY_KP_7            = 327,
    ILI_KEY_KP_8            = 328,
    ILI_KEY_KP_9            = 329,
    ILI_KEY_KP_DECIMAL      = 330,
    ILI_KEY_KP_DIVIDE       = 331,
    ILI_KEY_KP_MULTIPLY     = 332,
    ILI_KEY_KP_SUBTRACT     = 333,
    ILI_KEY_KP_ADD          = 334,
    ILI_KEY_KP_ENTER        = 335,
    ILI_KEY_KP_EQUAL        = 336,
    ILI_KEY_LEFT_SHIFT      = 340,
    ILI_KEY_LEFT_CONTROL    = 341,
    ILI_KEY_LEFT_ALT        = 342,
    ILI_KEY_LEFT_SUPER      = 343,
    ILI_KEY_RIGHT_SHIFT     = 344,
    ILI_KEY_RIGHT_CONTROL   = 345,
    ILI_KEY_RIGHT_ALT       = 346,
    ILI_KEY_RIGHT_SUPER     = 347,
    ILI_KEY_MENU            = 348,
    ILI_MOUSE_LEFT          = 512,
    ILI_MOUSE_RIGHT         = 513,
    ILI_MOUSE_MIDDLE        = 514,
    ILI_MOUSE_4             = 515,
    ILI_MOUSE_5             = 516,
    ILI_MOUSE_6             = 517,
    ILI_MOUSE_7             = 518,
    ILI_MOUSE_8             = 519,
    ILI_JOY_1               = 768,
    ILI_JOY_2               = 769,
    ILI_JOY_3               = 770,
    ILI_JOY_4               = 771,
    ILI_JOY_5               = 772,
    ILI_JOY_6               = 773,
    ILI_JOY_7               = 774,
    ILI_JOY_8               = 775,
    ILI_JOY_9               = 776,
    ILI_JOY_10              = 777,
    ILI_JOY_11              = 778,
    ILI_JOY_12              = 779,
    ILI_JOY_13              = 780,
    ILI_JOY_14              = 781,
    ILI_JOY_15              = 782,
    ILI_JOY_16              = 783,
};

/** Keyboard modifiers */
enum ilI_mod {
    ILI_MOD_SHIFT   = 0x1,
    ILI_MOD_CONTROL = 0x2,
    ILI_MOD_ALT     = 0x4,
    ILI_MOD_SUPER   = 0x8
};

/** Key/mouse button press/release */
typedef struct ilI_buttonevent {
    enum ilI_key button;
    int scancode;
    int device;
    int action;
    enum ilI_mod mods;
} ilI_buttonevent;

/** Returns whether the specified button is being pressed, and optionally returns which input backend reported it */
int ilI_getKey(enum ilI_key key, int *input);

typedef struct ilI_backend {
    char *name;
    int (*get)(struct ilI_backend *self, enum ilI_key key);
    void *user;
} ilI_backend;

/** Registers a new input backend, the memory must last until application exit */
int ilI_register(ilI_backend *backend);
/** Returns the name of the specified backend, allocated with strdup(3), the user must free it */
char *ilI_backend_getName(int input);

#endif

