#include "rmath.h"

void
viewPort(mat4 *arr, float x, float y, float width, float height)
{
	float depth = 255.0f;

	arr->elem[0 * 4 + 0] = width / 2.0f;
	arr->elem[0 * 4 + 3] = (x + width) / 2.0f;
	arr->elem[1 * 4 + 1] = height / 2.0f;
	arr->elem[1 * 4 + 3] = (y + height) / 2.0f;
	arr->elem[2 * 4 + 2] = depth / 2.0f;
	arr->elem[2 * 4 + 3] = depth / 2.0f;
}

void
screenSpaceTransform(mat4 *arr, float width, float height)
{
	arr->elem[0 * 4 + 0] = width;
	arr->elem[0 * 4 + 3] = width;
	arr->elem[1 * 4 + 1] = -height;
	arr->elem[1 * 4 + 3] = height;
}

void
quatToRotationMatrix(mat4 *m, vec4f *quat)
{
	float xx = quat->x * quat->x;
	float xy = quat->x * quat->y;
	float xz = quat->x * quat->z;
	float xw = quat->x * quat->w;

	float yy = quat->y * quat->y;
	float yz = quat->y * quat->z;
	float yw = quat->y * quat->w;

	float zz = quat->z * quat->z;
	float zw = quat->z * quat->w;

	m->elem[0 * 4 + 0] = yy + zz;
	m->elem[0 * 4 + 1] = xy - zw;
	m->elem[0 * 4 + 2] = xz + yw;

	m->elem[1 * 4 + 0] = xy + zw;
	m->elem[1 * 4 + 1] = xx + zz;
	m->elem[1 * 4 + 2] = yz - xw;

	m->elem[2 * 4 + 0] = xz - yw;
	m->elem[2 * 4 + 1] = yz + xw;
	m->elem[2 * 4 + 2] = xx + yy;

	__m128 two_4x = _mm_set_ps1(2.0f);

	__m128 m0 = _mm_load_ps(m->elem);
	m0 = _mm_mul_ps(m0, two_4x);
	_mm_store_ps(m->elem, m0);

	__m128 m1 = _mm_load_ps(m->elem + 4);
	m1 = _mm_mul_ps(m1, two_4x);
	_mm_store_ps(m->elem + 4, m1);

	__m128 m2 = _mm_load_ps(m->elem + 8);
	m2 = _mm_mul_ps(m2, two_4x);
	_mm_store_ps(m->elem + 8, m2);

	__m128 m3 = _mm_load_ps(m->elem + 12);
	m3 = _mm_mul_ps(m3, two_4x);
	_mm_store_ps(m->elem + 12, m3);

	m->elem[0 * 4 + 0] = 1.0f - m->elem[0 * 4 + 0];
	m->elem[1 * 4 + 1] = 1.0f - m->elem[1 * 4 + 1];
	m->elem[2 * 4 + 2] = 1.0f - m->elem[2 * 4 + 2];
}

void
setViewProgection(mat4 *proj, mat4 *pres, vec4f *quatRotate, vec3f *pos)
{
	*proj = identity();
	quatToRotationMatrix(proj, &conjugate(*quatRotate));

	mat4 trans = toTranslation(pos);
	*proj = *proj * trans;

	*proj = *pres * (*proj);
}

void
setModelTransformation(mat4 *m, vec4f *quatRotate, vec3f *pos)
{
	mat4 rot = identity();
	quatToRotationMatrix(&rot, quatRotate);

	*m = toTranslation(pos);

	//without scale (tran * rot * scale)
	*m = *m * rot;
}

void
setPerspective(mat4 *arr, float fov, float aspectRatio, float near, float far)
{
	float zRange = far - near;
	float tanHalfFov = tanf(fov / 2.0f);

	arr->elem[0 * 4 + 0] = 1.0f / (tanHalfFov * aspectRatio);
	arr->elem[1 * 4 + 1] = 1.0f / tanHalfFov;
	arr->elem[2 * 4 + 2] = -(near + far) / zRange;
	arr->elem[2 * 4 + 3] = -(2.0f * far * near) / zRange;
	arr->elem[3 * 4 + 2] = 1.0f;
}

void
setLookAtLH(mat4 *proj, mat4 *pres, vec3f *pos, vec3f *dir)
{
	vec3f up = { 0, 1.0f, 0 };
	*proj = identity();

	vec3f z = *dir - *pos;
	normalize(&z);
	vec3f x = cross(&up, &z);
	normalize(&x);
	vec3f y = cross(&z, &x);
	normalize(&y);

	*(vec3f *)proj->elem = x;
	*(vec3f *)(proj->elem + 4) = y;
	*(vec3f *)(proj->elem + 8) = z;
	
	proj->elem[0 * 4 + 3] = -dot(&x, pos);
	proj->elem[1 * 4 + 3] = -dot(&y, pos);
	proj->elem[2 * 4 + 3] = -dot(&z, pos);

	*proj = *pres * (*proj);
}


void
setLookAtRH(mat4 *proj, mat4 *pres, vec3f *pos, vec3f *dir)
{
	vec3f up = { 0, 1.0f, 0 };
	*proj = identity();

	vec3f z = *dir - *pos;
	normalize(&z);
	vec3f x = cross(&up, &z);
	normalize(&x);
	vec3f y = cross(&z, &x);
	normalize(&y);

	*(vec3f *)proj->elem = x;
	*(vec3f *)(proj->elem + 4) = y;
	*(vec3f *)(proj->elem + 8) = z;

	//dot() must not be "negative"
	proj->elem[0 * 4 + 3] = -dot(&x, pos);
	proj->elem[1 * 4 + 3] = -dot(&y, pos);
	proj->elem[2 * 4 + 3] = -dot(&z, pos);

	*proj = *pres * (*proj);
}

mat4
XRotation(float angel)
{
	mat4 mat = identity();
	float s = sin(angel);
	float c = cos(angel);
	
	mat.elem[1 * 4 + 1] = c;
	mat.elem[1 * 4 + 2] = -s;
	mat.elem[2 * 4 + 1] = s;
	mat.elem[2 * 4 + 2] = c;

	return mat;
}

mat4
YRotation(float angel)
{
	mat4 mat = identity();
	float s = sin(angel);
	float c = cos(angel);

	mat.elem[0 * 4 + 0] = c;
	mat.elem[0 * 4 + 2] = s;
	mat.elem[2 * 4 + 0] = -s;
	mat.elem[2 * 4 + 2] = c;

	return mat;
}

mat4
ZRotation(float angel)
{
	mat4 mat = identity();
	float s = sin(angel);
	float c = cos(angel);

	mat.elem[0 * 4 + 0] = c;
	mat.elem[0 * 4 + 1] = -s;
	mat.elem[1 * 4 + 0] = s;
	mat.elem[1 * 4 + 1] = c;

	return mat;
}

vec4f
quatMulQuat(vec4f *a, vec4f *b)
{
	vec4f result;
	result.x = (a->w * b->x) + (a->x * b->w) + (a->y * b->z) - (a->z * b->y);
	result.y = (a->w * b->y) - (a->x * b->z) + (a->y * b->w) + (a->z * b->x);
	result.z = (a->w * b->z) + (a->x * b->y) - (a->y * b->x) + (a->z * b->w);
	result.w = (a->w * b->w) - (a->x * b->x) - (a->y * b->y) - (a->z * b->z);
	return result;
}

vec4f
quatMulVec(vec4f *a, vec3f *b)
{
	vec4f result;
	result.w = (-a->x * b->x) - (a->y * b->y) - (a->z * b->z);
	result.x = (a->w * b->x) + (a->y * b->z) - (a->z * b->y);
	result.y = (a->w * b->y) - (a->x * b->z) + (a->z * b->x);
	result.z = (a->w * b->z) + (a->x * b->y) - (a->y * b->x);
	return result;
}

inline vec4f
preperaQuat(vec3f v, float angel)
{
	vec4f quat;
	angel *= 0.5f;
	float s = sin(angel);
	quat.xyz = v;
	quat *= s;
	quat.w = cos(angel);
	return quat;
}

void
rotateTransformQuat(vec4f *quat, float angelX, float angelY)
{
	vec3f rotAxis = { 0, 1.0f, 0 };
	vec4f rotQuat = preperaQuat(rotAxis, angelX);
	*quat = quatMulQuat(&rotQuat, quat);
	normalize(quat);

	vec4f cunj = conjugate(*quat);
	rotAxis = { 1.0f, 0, 0 };
	rotQuat = quatMulVec(quat, &rotAxis);
	rotQuat = quatMulQuat(&rotQuat, &cunj);
	rotAxis = rotQuat.xyz;
	rotQuat = preperaQuat(rotAxis, angelY);
	*quat = quatMulQuat(&rotQuat, quat);
	normalize(quat);
}

void
rotateTransform2(vec3f* vec, float yaw, float pitch)
{
	float cosY = cosf(yaw);
	float sinY = sinf(yaw);
	float cosP = cosf(pitch);

	vec->y = sinf(pitch);
	vec->x = cosY * cosP;
	vec->z = sinY * cosP;
	normalize(vec);
}

//fix lock y-axis
void
rotateTransform(vec3f *vec, float angelX, float angelY)
{
	vec3f rotAxis = {0, 1.0f, 0};
	vec4f quat = preperaQuat(rotAxis, angelX);
	vec4f conj = conjugate(quat);
	vec4f quat2 = quatMulVec(&quat, vec);
	quat2 = quatMulQuat(&quat2, &conj);
	*vec = quat2.xyz;

	vec3f z = quat2.xyz;
	normalize(&z);
	rotAxis = cross(&rotAxis, &z);
	normalize(&rotAxis);

	quat = preperaQuat(rotAxis, angelY);
	conj = conjugate(quat);
	quat2 = quatMulVec(&quat, vec);
	quat2 = quatMulQuat(&quat2, &conj);
	*vec = quat2.xyz;
}

void 
transpose(mat3 *m)
{
	float tmp;

	tmp = m->elem[0 * 3 + 1];
	m->elem[0 * 3 + 1] = m->elem[1 * 3 + 0];
	m->elem[1 * 3 + 0] = tmp;

	tmp = m->elem[0 * 3 + 2];
	m->elem[0 * 3 + 2] = m->elem[2 * 3 + 0];
	m->elem[2 * 3 + 0] = tmp;

	tmp = m->elem[1 * 3 + 2];
	m->elem[1 * 3 + 2] = m->elem[2 * 3 + 1];
	m->elem[2 * 3 + 1] = tmp;
}