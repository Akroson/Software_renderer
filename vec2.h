#pragma once

struct vec2f
{
	union
	{
		float elem[2];

		struct
		{
			float x;
			float y;
		};
	};
};

inline float
cross(vec2f a, vec2f b)
{
	return (a.x * b.y) - (b.x * a.y);
}

inline vec2f
operator-(vec2f &a, vec2f &b)
{
	vec2f result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	return result;
}

inline void
operator*=(vec2f &a, vec2f &b)
{
	a.x *= b.x;
	a.y *= b.y;
}


inline void
operator*=(vec2f &a, float b)
{
	a.x *= b;
	a.y *= b;
}

inline void
operator+=(vec2f &a, vec2f &b)
{
	a.x += b.x;
	a.y += b.y;
}