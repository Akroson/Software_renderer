#pragma once

#include "vec3.h"
#include "vec4.h"

//4x4
//row-major
struct mat4
{
	float elem[16];
};

inline mat4
operator*(mat4 &a, mat4 &b)
{
	mat4 result = {};

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			float sum = 0.0f;
			for (int k = 0; k < 4; k++)
			{
				sum += a.elem[i * 4 + k] * b.elem[k * 4 + j];
			}
			result.elem[i * 4 + j] = sum;
		}
	}

	return result;
}

inline vec4f
operator*(mat4 &m, vec4f &v)
{
	vec4f result;

	__m128 m0 = _mm_loadu_ps(m.elem);
	__m128 m1 = _mm_loadu_ps(m.elem + 4);
	__m128 m2 = _mm_loadu_ps(m.elem + 8);
	__m128 m3 = _mm_loadu_ps(m.elem + 12);

	__m128 r0 = _mm_mul_ps(m0, v.xmm);
	__m128 r1 = _mm_mul_ps(m1, v.xmm);
	__m128 r2 = _mm_mul_ps(m2, v.xmm);
	__m128 r3 = _mm_mul_ps(m3, v.xmm);

	__m128 sum0 = _mm_hadd_ps(r0, r1);
	__m128 sum1 = _mm_hadd_ps(r2, r3);

	result.xmm = _mm_hadd_ps(sum0, sum1);

	return result;
}

inline vec3f
operator*(mat4 &m, vec3f &v)
{
	vec3f result;
	result.x = (m.elem[0 * 4 + 0] * v.x) + (m.elem[0 * 4 + 1] * v.y) + (m.elem[0 * 4 + 2] * v.z) + m.elem[0 * 4 + 3];// * 1.0f;
	result.y = (m.elem[1 * 4 + 0] * v.x) + (m.elem[1 * 4 + 1] * v.y) + (m.elem[1 * 4 + 2] * v.z) + m.elem[1 * 4 + 3];
	result.z = (m.elem[2 * 4 + 0] * v.x) + (m.elem[2 * 4 + 1] * v.y) + (m.elem[2 * 4 + 2] * v.z) + m.elem[2 * 4 + 3];
	return result;
}

inline mat4
identity(void)
{ 
	mat4 arr = {};
	arr.elem[0 * 4 + 0] = 1;
	arr.elem[1 * 4 + 1] = 1;
	arr.elem[2 * 4 + 2] = 1;
	arr.elem[3 * 4 + 3] = 1;
	return arr;
}