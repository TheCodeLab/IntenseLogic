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
#include "common/positionable.h"

ilG_camera* ilG_camera_new(il_positionable * parent)
{
    ilG_camera* camera = calloc(1, sizeof(ilG_camera));
    camera->positionable = parent;
    camera->projection_matrix = il_Matrix_identity;
    camera->sensitivity = 0.002;
    camera->refs = 1;
    return camera;
}

struct ctx {
    ilG_camera* camera;
    il_keymap* keymap;
    int first_mouse;
};

static void handleMouseMove(ilE_queue* queue, ilE_event* ev, struct ctx * ctx)
{
    (void)queue;
    ilI_mouseMove * mousemove = (ilI_mouseMove*)ilE_getData(ev, NULL);

    if (!ilI_isButtonSet(GLFW_MOUSE_BUTTON_1)) {
        ctx->first_mouse = 1;
        return;
    }
    // ignore the first mouse move because of weird behaviour on windows
    if (ctx->first_mouse) {
        ctx->first_mouse = 0;
        return;
    }

    il_log(5, "MouseMove: %i %i", mousemove->x, mousemove->y);

    il_Quaternion yaw = il_Quaternion_fromAxisAngle(
    (il_Vector3) {
        0, 1, 0
    },
    -mousemove->x * ctx->camera->sensitivity
                        );

    il_Quaternion pitch = il_Quaternion_fromAxisAngle(
    (il_Vector3) {
        1, 0, 0
    },
    -mousemove->y * ctx->camera->sensitivity
                          );

    il_Quaternion quat;

    quat = il_Quaternion_mul(ctx->camera->positionable->rotation, yaw);
    quat = il_Quaternion_mul(pitch, quat);

    ctx->camera->positionable->rotation = quat;

}

static void handleTick(ilE_queue* queue, ilE_event* ev, struct ctx * ctx)
{
    (void)queue, (void)ctx, (void)ev;
    int forward   = ilI_isKeySet(il_keymap_getkey(ctx->keymap->camera_backward))
                    - ilI_isKeySet(il_keymap_getkey(ctx->keymap->camera_forward));
    int leftward  = ilI_isKeySet(il_keymap_getkey(ctx->keymap->camera_right))
                    - ilI_isKeySet(il_keymap_getkey(ctx->keymap->camera_left));
    int upward    = ilI_isKeySet(il_keymap_getkey(ctx->keymap->camera_up))
                    - ilI_isKeySet(il_keymap_getkey(ctx->keymap->camera_down));

    //il_log(5, "Camera moving for=%i lef=%i upw=%i", forward, leftward, upward);

    if (forward == 0 && leftward == 0 && upward == 0) return;
    il_log(5, "Moving camera.");

    il_positionable_translate (
        ctx->camera->positionable,
    (il_Vector3) {
        ctx->camera->movespeed.x * -leftward,
            ctx->camera->movespeed.y * -upward,
            ctx->camera->movespeed.z * forward
    }
    );
}


static void mousedown(ilE_queue* queue, ilE_event* ev, int ctx)
{
    (void)queue, (void)ev, (void)ctx;
    ilI_grabMouse(1);	// Grab input and hide cursor
}

static void mouseup(ilE_queue* queue, ilE_event* ev, int ctx)
{
    (void)queue, (void)ev, (void)ctx;
    ilI_grabMouse(0);	// Release input and show mouse again
}

void ilG_camera_setEgoCamKeyHandlers(ilG_camera* camera, struct il_keymap * keymap)
{
    struct ctx * ctx = calloc(1, sizeof(struct ctx));
    ctx->camera = camera;
    ctx->keymap = keymap;
    ctx->first_mouse = 1;
    ilE_register(il_queue, IL_BASE_TICK,        ILE_DONTCARE, (ilE_callback)&handleTick,      ctx);
    ilE_register(il_queue, IL_INPUT_MOUSEMOVE,  ILE_DONTCARE, (ilE_callback)&handleMouseMove, ctx);
    ilE_register(il_queue, IL_INPUT_MOUSEDOWN,  ILE_DONTCARE, (ilE_callback)&mousedown,       NULL);
    ilE_register(il_queue, IL_INPUT_MOUSEUP,    ILE_DONTCARE, (ilE_callback)&mouseup,         NULL);
}

void ilG_camera_setMovespeed(ilG_camera* camera, il_Vector3 movespeed, float pixels_per_radian)
{
    camera->movespeed = movespeed;
    camera->sensitivity = 1.0/pixels_per_radian;
}
