/** @file positionable.h
 * @brief Objects that can be positioned in the world
 */

#ifndef IL_COMMON_POSITIONABLE_H
#define IL_COMMON_POSITIONABLE_H

#include <sys/time.h>

#include "math/quaternion.h"
#include "math/vector.h"
#include "common/world.h"
#include "common/base.h"

extern il_type il_positionable_type;

/** @deprecated Going to be replaced when the scenegraph is linearized */
typedef struct il_positionable {
    il_base base;
    il_vec3 position;
    il_quat rotation;
    il_vec3 size;
    il_vec3 velocity;
    struct timeval last_update;         /**< Unused */
    struct ilG_drawable3d* drawable;
    struct ilG_material* material;
    struct ilG_texture* texture;
} il_positionable;

il_positionable * il_positionable_new();

/** Translates a positionable relative to its rotation */
void il_positionable_translate(il_positionable*, float x, float y, float z);

#endif
