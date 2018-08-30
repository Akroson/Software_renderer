#pragma once
#include <stdint.h>
#include <math.h>
#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;

#include "vec2.h"
#include "vec3.h"
#include "vec4.h"
#include "mat3.h"
#include "mat4.h"

#define PI 3.14159265359

void
viewPort(mat4 *arr, float x, float y, float width, float height);

void
setLookAtLH(mat4 *proj, mat4 *pres, vec3f *pos, vec3f *dir);

void
setLookAtRH(mat4 *proj, mat4 *pres, vec3f *pos, vec3f *dir);

void
screenSpaceTransform(mat4 *arr, float width, float height);

void
setPerspective(mat4 *arr, float fov, float aspectRatio, float near, float far);

void
setViewProgection(mat4 *proj, mat4* presp, vec4f *rotate, vec3f *pos);

void
setModelTransformation(mat4 *m, vec4f *quatRotate, vec3f *pos);

void
quatToRotationMatrix(mat4 *m, vec4f *quat);

void
rotateTransform(vec3f *vec, float angelX, float angelY);

void
rotateTransform2(vec3f* vec, float yaw, float pitch);

void
rotateTransformQuat(vec4f *quat, float angelX, float angelY);

vec4f
quatMulQuat(vec4f *a, vec4f *b);

vec4f
quatMulVec(vec4f *a, vec3f *b);

void
transpose(mat3 *m);

inline vec4f
conjugate(vec4f q)
{
	vec4f result;
	result = q * -1.0f;
	result.w = q.w;
	return result;
}

inline mat4
toTranslation(vec3f *pos)
{
	mat4 trans = identity();

	trans.elem[0 * 4 + 3] = pos->x;
	trans.elem[1 * 4 + 3] = pos->y;
	trans.elem[2 * 4 + 3] = pos->z;

	return trans;
}