#ifndef ILG_SCREENSHOT_H
#define ILG_SCREENSHOT_H

#include "renderer.h"

struct ilA_img;
struct ilG_context;

/** cb will only be called once */
ilG_builder ilG_grabber_builder(struct ilG_context *context, void (*cb)(struct ilA_img res, void *user), void *user);

#endif
