#pragma once

struct vec4i
{
	union
	{
		int elem[4];

		struct
		{
			union
			{
				vec3i xyz;
				struct
				{
					int x;
					int y;
					int z;
				};
			};
			int w;
		};

		struct
		{
			__m128i xmm;
		};
	};
};

struct vec4f
{
	union
	{
		float elem[4];

		struct 
		{
			union
			{
				vec3f xyz;
				struct
				{
					float x;
					float y;
					float z;
				};
			};
			float w;
		};

		struct
		{
			__m128 xmm;
		};
	};
};


inline void
operator+=(vec4f &a, vec4f &b)
{
	a.xmm = _mm_add_ps(a.xmm, b.xmm);
}

inline void
operator+=(vec4f &a, float b)
{
	__m128 b_4x = _mm_set_ps1(b);
	a.xmm = _mm_add_ps(a.xmm, b_4x);
}

inline vec4f
operator*(vec4f &a, float b)
{
	vec4f result;
	__m128 b_4x = _mm_set_ps1(b);
	result.xmm = _mm_mul_ps(a.xmm, b_4x);
	return result;
}

inline void
operator*=(vec4f &a, float b)
{
	__m128 b_4x = _mm_set_ps1(b);
	a.xmm = _mm_mul_ps(a.xmm, b_4x);
}

inline vec4f
operator/(vec4f &a, vec4f &b)
{
	vec4f result;
	result.xmm = _mm_div_ps(a.xmm, b.xmm);
	return result;
}

inline void
operator/=(vec4f &a, float b)
{
	__m128 b_4x = _mm_set_ps1(b);
	a.xmm = _mm_div_ps(a.xmm, b_4x);
}

inline vec4f
operator-(vec4f &a, vec4f &b)
{
	vec4f result;
	result.xmm = _mm_sub_ps(a.xmm, b.xmm);
	return result;
}

inline void
operator-=(vec4f &a, vec4f &b)
{
	a.xmm = _mm_sub_ps(a.xmm, b.xmm);
}

inline vec4f
operator-(vec4f &a, float b)
{
	vec4f result;
	__m128 b_4x = _mm_set_ps1(b);
	result.xmm = _mm_sub_ps(a.xmm, b_4x);
	return result;
}

inline float
lengthVec(vec4f *v)
{
	float num = (v->x * v->x) + (v->y * v->y) + (v->z * v->z) + (v->w * v->w);
	return sqrtf(num);
}

inline void
normalize(vec4f *v)
{
	float x = lengthVec(v);
	x = 1.0f / x;
	*v *= x;
}
