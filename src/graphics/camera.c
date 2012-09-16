#include "camera.h"

#include <stdlib.h>
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include "common/keymap.h"
#include "common/event.h"
#include "common/base.h"
#include "common/input.h"
#include "common/keymap.h"
#include "common/log.h"

il_Graphics_Camera* il_Graphics_Camera_new(il_Common_Positionable * parent) {
  il_Graphics_Camera* camera = calloc(sizeof(il_Graphics_Camera),1);
  camera->positionable = parent;
  camera->refs = 1;
  return camera;
}

struct ctx {
  il_Graphics_Camera* camera;
  il_Common_Keymap* keymap;
};

static void handleMouseMove(il_Event_Event* ev, struct ctx * ctx) {
  il_Input_MouseMove * mousemove = (il_Input_MouseMove*)ev->data;
}

static void handleTick(il_Event_Event* ev, struct ctx * ctx) {
  int forward   = il_Input_isKeySet(il_Common_Keymap_getkey(ctx->keymap->camera_backward)) 
                - il_Input_isKeySet(il_Common_Keymap_getkey(ctx->keymap->camera_forward));
  int leftward  = il_Input_isKeySet(il_Common_Keymap_getkey(ctx->keymap->camera_right)) 
                - il_Input_isKeySet(il_Common_Keymap_getkey(ctx->keymap->camera_left));
  int upward    = il_Input_isKeySet(il_Common_Keymap_getkey(ctx->keymap->camera_up)) 
                - il_Input_isKeySet(il_Common_Keymap_getkey(ctx->keymap->camera_down));
  
  if (forward == 0 && leftward == 0 && upward == 0) return;
  il_Common_log(5, "Moving camera.\n");
  
  il_Graphics_Camera_translate ( 
    ctx->camera, 
    (sg_Vector3) {
      ctx->camera->movespeed.x * leftward, 
      ctx->camera->movespeed.y * upward, 
      ctx->camera->movespeed.z * forward
    }
  );
}

void il_Graphics_Camera_setEgoCamKeyHandlers(il_Graphics_Camera* camera, il_Common_Keymap * keymap) {
  struct ctx * ctx = malloc(sizeof(struct ctx));
  ctx->camera = camera;
  ctx->keymap = keymap;
  il_Event_register(IL_BASE_TICK, (il_Event_Callback)&handleTick, ctx);
  il_Event_register(IL_INPUT_MOUSEMOVE, (il_Event_Callback)&handleMouseMove, ctx);
}

void il_Graphics_Camera_render(il_Graphics_Camera* camera) {
  glTranslatef(-camera->positionable->position.x, -camera->positionable->position.y, -camera->positionable->position.z);
}

void il_Graphics_Camera_setMovespeed(il_Graphics_Camera* camera, sg_Vector3 movespeed, float radians_per_pixel) {
  camera->movespeed = movespeed;
  
}

void il_Graphics_Camera_translate(il_Graphics_Camera* camera, sg_Vector3 vec) {
  il_Common_log(5, "Translating camera by (%f, %f, %f)\n", vec.x, vec.y, vec.z);
  sg_Vector3 res = sg_Vector3_rotate(vec, camera->positionable->rotation);
  camera->positionable->position.x += res.x;
  camera->positionable->position.y += res.y;
  camera->positionable->position.z += res.z;
}
