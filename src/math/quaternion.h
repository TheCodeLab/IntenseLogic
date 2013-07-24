/** @file quaternion.h
 * @brief Quaternion operations
 */

#ifndef IL_QUATERNION_H
#define IL_QUATERNION_H

typedef float *il_quat;

il_quat il_quat_new();
void il_quat_free(il_quat q);
il_quat il_quat_copy(il_quat q);

il_quat il_quat_set(il_quat q, float x, float y, float z, float w);
il_quat il_quat_mul(const il_quat a, const il_quat b, il_quat q);
/** Linear interpolation between two quaternions, using the t coefficient from 0 to 1 */
il_quat il_quat_lerp(const il_quat a, const il_quat b, float t, il_quat q);
/** Converts Euler angles to a quaternion */
il_quat il_quat_fromYPR(float yaw, float pitch, float roll, il_quat q);
/** Converts to quaternion from Axis-Angle representation */
il_quat il_quat_fromAxisAngle(float x, float y, float z, float a, il_quat q);
/** Normalizes a quaternion to keep it as a rotation */
il_quat il_quat_normalize(const il_quat a, il_quat q);
float il_quat_len(const il_quat a);
float il_quat_dot(const il_quat a, const il_quat b);
char *il_quat_print(const il_quat q, char *buf, unsigned len);

#endif

