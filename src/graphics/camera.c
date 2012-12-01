#include "camera.h"

#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glfw.h>

#include "common/keymap.h"
#include "common/event.h"
#include "common/base.h"
#include "common/input.h"
#include "common/keymap.h"
#include "common/log.h"

il_Graphics_Camera* il_Graphics_Camera_new(il_Common_Positionable * parent) {
  il_Graphics_Camera* camera = calloc(1, sizeof(il_Graphics_Camera));
  camera->positionable = parent;
  camera->projection_matrix = sg_Matrix_identity;
  camera->sensitivity = 0.002;
  camera->refs = 1;
  return camera;
}

struct ctx {
  il_Graphics_Camera* camera;
  il_Common_Keymap* keymap;
  int first_mouse;
};

static void handleMouseMove(il_Event_Event* ev, struct ctx * ctx) 
{
  il_Input_MouseMove * mousemove = (il_Input_MouseMove*)ev->data;
  
  if (!il_Input_isButtonSet(GLFW_MOUSE_BUTTON_1)) {
    ctx->first_mouse = 1;
    return;
  }
  // ignore the first mouse move because of weird behaviour on windows
  if (ctx->first_mouse) {
    ctx->first_mouse = 0;
    return;
  }
  
  il_Common_log(5, "MouseMove: %i %i", mousemove->x, mousemove->y);
  
  sg_Quaternion yaw = sg_Quaternion_fromAxisAngle(
    (sg_Vector3) {0, 1, 0},
    -mousemove->x * ctx->camera->sensitivity
  );
  
  sg_Quaternion pitch = sg_Quaternion_fromAxisAngle(
    (sg_Vector3) {1, 0, 0},
    -mousemove->y * ctx->camera->sensitivity
  );
  
  sg_Quaternion quat;
  
  quat = sg_Quaternion_mul(ctx->camera->positionable->rotation, yaw);
  quat = sg_Quaternion_mul(pitch, quat);

  ctx->camera->positionable->rotation = quat;
  
}

static void handleTick(il_Event_Event* ev, struct ctx * ctx) {
  (void)ctx;
  (void)ev;
  int forward   = il_Input_isKeySet(il_Common_Keymap_getkey(ctx->keymap->camera_backward)) 
                - il_Input_isKeySet(il_Common_Keymap_getkey(ctx->keymap->camera_forward));
  int leftward  = il_Input_isKeySet(il_Common_Keymap_getkey(ctx->keymap->camera_right)) 
                - il_Input_isKeySet(il_Common_Keymap_getkey(ctx->keymap->camera_left));
  int upward    = il_Input_isKeySet(il_Common_Keymap_getkey(ctx->keymap->camera_up)) 
                - il_Input_isKeySet(il_Common_Keymap_getkey(ctx->keymap->camera_down));
  
  //il_Common_log(5, "Camera moving for=%i lef=%i upw=%i", forward, leftward, upward);
  
  if (forward == 0 && leftward == 0 && upward == 0) return;
  il_Common_log(5, "Moving camera.");
  
  il_Common_Positionable_translate ( 
    ctx->camera->positionable, 
    (sg_Vector3) {
      ctx->camera->movespeed.x * -leftward, 
      ctx->camera->movespeed.y * -upward, 
      ctx->camera->movespeed.z * forward
    }
  );
}


static void mousedown(il_Event_Event* ev, int ctx){
  (void)ev;
  (void)ctx;
	il_Input_GrabMouse(1);	// Grab input and hide cursor
}

static void mouseup(il_Event_Event* ev, int ctx){
  (void)ev;
  (void)ctx;
	il_Input_GrabMouse(0);	// Release input and show mouse again
}

void il_Graphics_Camera_setEgoCamKeyHandlers(il_Graphics_Camera* camera, il_Common_Keymap * keymap) {
  struct ctx * ctx = calloc(1, sizeof(struct ctx));
  ctx->camera = camera;
  ctx->keymap = keymap;
  il_Event_register(IL_BASE_TICK, (il_Event_Callback)&handleTick, ctx);
  il_Event_register(IL_INPUT_MOUSEMOVE, (il_Event_Callback)&handleMouseMove, ctx);
  il_Event_register(IL_INPUT_MOUSEDOWN, (il_Event_Callback)&mousedown, NULL);
  il_Event_register(IL_INPUT_MOUSEUP, (il_Event_Callback)&mouseup, NULL);

}

void il_Graphics_Camera_setMovespeed(il_Graphics_Camera* camera, sg_Vector3 movespeed, float pixels_per_radian) {
  camera->movespeed = movespeed;
  camera->sensitivity = 1.0/pixels_per_radian;
}
