#ifndef ILG_SCREENSHOT_H
#define ILG_SCREENSHOT_H

#include "renderer.h"

struct ilA_img;

/** cb will only be called once, and is responsible for cleaning up itself and removing the grabber from the scenegraph. */
ilG_builder ilG_grabber_builder(void (*cb)(struct ilA_img* res, void *user), void *user);

#endif
