#pragma once
#include <math.h>
#include "vec2.h"

struct vec3i
{
	union
	{
		int elem[3];

		struct
		{
			int x, y, z;
		};
	};
};

struct vec3f
{
	union
	{
		float elem[3];

		struct
		{
			union
			{
				vec2f xy;
				struct
				{
					float x;
					float y;
				};
			};
			float z;
		};
	};
};

inline void
operator-=(vec3i &a, int b)
{
	a.x -= b;
	a.y -= b;
	a.z -= b;
}

inline vec3f
operator-(vec3f a, vec3f b)
{
	vec3f result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	return result;
}

inline vec3f
operator*(vec3f a, float b)
{
	vec3f result;
	result.x = a.x * b;
	result.y = a.y * b;
	result.z = a.z * b;
	return result;
}

inline vec3f
operator+(vec3f &a, vec3f &b)
{
	vec3f result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
	return result;
}

inline void
operator+=(vec3f &a, float b)
{
	a.x += b;
	a.y += b;
	a.z += b;
}

inline void
operator+=(vec3f &a, vec3f &b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
}

inline void
operator-=(vec3f &a, float b)
{
	a.x -= b;
	a.y -= b;
	a.z -= b;
}

inline void
operator-=(vec3f &a, vec3f &b)
{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
}

inline void
operator*=(vec3f &a, float b)
{
	a.x *= b;
	a.y *= b;
	a.z *= b;
}

inline void
operator/=(vec3f &a, float b)
{
	a.x /= b;
	a.y /= b;
	a.z /= b;
}

inline int
lengthVec(vec3i *v)
{
	int num = (v->x * v->x) + (v->y * v->y) + (v->z * v->z);
	return sqrt(num);
}

inline float
lengthVec(vec3f *v)
{
	float num = (v->x * v->x) + (v->y * v->y) + (v->z * v->z);
	return sqrtf(num);
}

inline float
dot(vec3f *a, vec3f *b)
{
	return (a->x * b->x) + (a->y * b->y) + (a->z * b->z);
}

inline void
normalize(vec3f *v)
{
	float x = lengthVec(v);
	x = 1.0f / x;
	*v *= x;
}

inline vec3f
cross(vec3f *a, vec3f *b)
{
	vec3f result;
	result.x = (a->y * b->z) - (a->z * b->y);
	result.y = (a->z * b->x) - (a->x * b->z);
	result.z = (a->x * b->y) - (a->y * b->x);
	return result;
}