#pragma once

#include "vec3.h"

//3x3
//row-major
struct mat3
{
	float elem[9];
};


inline vec3f
operator*(mat3 &a, vec3f &b)
{
	vec3f result;
	vec4f tmpVec;
	vec4f tmpMat;

	tmpVec.xyz = b;

	for (int i = 0; i < 3; i++)
	{
		tmpMat.xyz = *((vec3f *)&a + i);
		tmpMat.xmm = _mm_mul_ps(tmpMat.xmm, tmpVec.xmm);
		result.elem[i] = tmpMat.x + tmpMat.y + tmpMat.z;
	}

	return result;
}