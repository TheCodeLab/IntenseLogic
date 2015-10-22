/** @file quaternion.h
 * @brief Quaternion operations
 */

#ifndef IL_QUATERNION_H
#define IL_QUATERNION_H

typedef struct il_quat {
  float x, y, z, w;
} il_quat;

il_quat il_quat_new(float x, float y, float z, float w);
il_quat il_quat_mul(il_quat a, il_quat b);
/** Linear interpolation between two quaternions, using the t coefficient from 0 to 1 */
il_quat il_quat_lerp(il_quat a, il_quat b, float t);
/** Converts Euler angles to a quaternion */
il_quat il_quat_fromYPR(float yaw, float pitch, float roll);
/** Converts to quaternion from Axis-Angle representation */
il_quat il_quat_fromAxisAngle(float x, float y, float z, float a);
/** Normalizes a quaternion to keep it as a rotation */
il_quat il_quat_normalize(il_quat a);
il_quat il_quat_invert(il_quat q);
float il_quat_len(il_quat a);
float il_quat_dot(il_quat a, il_quat b);
char *il_quat_print(il_quat q, char *buf, unsigned len);

#endif
