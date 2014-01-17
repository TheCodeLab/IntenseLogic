/** @file quaternion.h
 * @brief Quaternion operations
 */

#ifndef IL_QUATERNION_H
#define IL_QUATERNION_H

typedef struct il_quat {
  float x, y, z, w;
}  __attribute__((aligned(16), packed)) il_quat;

il_quat il_quat_new(float x, float y, float z, float w);
il_quat il_quat_mul(const il_quat a, const il_quat b);
/** Linear interpolation between two quaternions, using the t coefficient from 0 to 1 */
il_quat il_quat_lerp(const il_quat a, const il_quat b, float t);
/** Converts Euler angles to a quaternion */
il_quat il_quat_fromYPR(float yaw, float pitch, float roll);
/** Converts to quaternion from Axis-Angle representation */
il_quat il_quat_fromAxisAngle(float x, float y, float z, float a);
/** Normalizes a quaternion to keep it as a rotation */
il_quat il_quat_normalize(const il_quat a);
float il_quat_len(const il_quat a);
float il_quat_dot(const il_quat a, const il_quat b);
char *il_quat_print(const il_quat q, char *buf, unsigned len);

#endif

