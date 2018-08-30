//#include "matrix.h"
//
//void
//viewPort(mat4 *arr, float x, float y, float width, float height)
//{
//	float depth = 255.0f;
//
//	arr->elem[0 * 4 + 0] = width / 2.0f;
//	arr->elem[0 * 4 + 3] = (x + width) / 2.0f;
//	arr->elem[1 * 4 + 1] = height / 2.0f;
//	arr->elem[1 * 4 + 3] = (y + height) / 2.0f;
//	arr->elem[2 * 4 + 2] = depth / 2.0f;
//	arr->elem[2 * 4 + 3] = depth / 2.0f;
//}
//
//void
//prepareLookAt(mat4 *mat, vec3f *camera)
//{
//	vec3f center = { 0, 0, 0 };
//	vec3f up = { 0, 1.0f, 0 };
//
//	vec3f z = *camera - center;//(camera - center)
//	normalize(&z);
//	vec3f x = cross(&up, &z);
//	normalize(&x);
//	vec3f y = cross(&z, &x);
//	normalize(&y);
//
//	for (int i = 0; i < 3; i++)
//	{
//		mat->elem[0 * 4 + i] = x.elem[i];
//		mat->elem[1 * 4 + i] = y.elem[i];
//		mat->elem[2 * 4 + i] = z.elem[i];
//	}
//
//	mat->elem[0 * 4 + 3] = -dot(&x, camera);
//	mat->elem[1 * 4 + 3] = -dot(&y, camera);
//	mat->elem[2 * 4 + 3] = -dot(&z, camera);
//}
//
//mat4
//XRotation(float angel)
//{
//	float s = sin(angel);
//	float c = cos(angel);
//
//	mat4 mat =
//	{
//		1, 0,  0, 0,
//		0, c, -s, 0,
//		0, s,  c, 0,
//		0, 0,  0, 1
//	};
//
//	return mat;
//}
//
//mat4
//YRotation(float angel)
//{
//	float s = sin(angel);
//	float c = cos(angel);
//
//	mat4 mat =
//	{
//		 c, 0, s, 0,
//		 0, 1, 0, 0,
//		-s, 0, c, 0,
//		 0, 0, 0, 1
//	};
//
//	return mat;
//}
//
//mat4
//ZRotation(float angel)
//{
//	float s = sin(angel);
//	float c = cos(angel);
//
//	mat4 mat =
//	{
//		c, -s, 0, 0,
//		s,  c, 0, 0,
//		0,  0, 0, 0,
//		0,  0, 0, 0
//	};
//
//	return mat;
//}
//
//vec4f
//quatMulQuat(vec4f *a, vec4f *b)
//{
//	vec4f result;
//	result.w = (a->w * b->w) - (a->x * b->x) - (a->y * b->y) - (a->z * b->z);
//	result.x = (a->w * b->x) + (a->x * b->w) + (a->y * b->z) - (a->z * b->y);
//	result.y = (a->w * b->y) - (a->x * b->z) + (a->y * b->w) + (a->z * b->x);
//	result.z = (a->w * b->z) + (a->x * b->y) - (a->y * b->x) + (a->z * b->w);
//	return result;
//}
//
//vec4f
//quatMulVec(vec4f *a, vec3f *b)
//{
//	vec4f result;
//	result.w = (-a->x * b->x) - (a->y * b->y) - (a->z * b->z);
//	result.x = (a->w * b->x) + (a->y * b->z) - (a->z * b->y);
//	result.y = (a->w * b->y) - (a->x * b->z) + (a->z * b->x);
//	result.z = (a->w * b->z) + (a->x * b->y) - (a->y * b->x);
//	return result;
//}
//
//vec4f
//quatInvert(vec4f q)
//{
//	q.x = -q.x;
//	q.y = -q.y;
//	q.z = -q.z;
//	q.w = q.w;
//	normalize(&q);
//	return q;
//}
//
//inline vec4f
//preperaQuat(vec3f v, float angel)
//{
//	vec4f quat;
//	angel *= 0.5f;
//	float s = sin(angel);
//	quat.w = cos(angel);
//	quat.x = v.x * s;
//	quat.y = v.y * s;
//	quat.z = v.z * s;
//	return quat;
//}
//
//void
//rotateTransform(vec3f *vec, float angelX, float angelY)
//{
//	vec4f quat;
//	vec3f rotAxis = { 0,1.0f,0 };
//	float halfAngel = angelX * 0.5f;
//	float s = sin(halfAngel);
//	quat.w = cos(halfAngel);
//	quat.x = 0;
//	quat.y = rotAxis.y * s;
//	quat.z = 0;
//	//quat = preperaQuat(norm, angelX);
//
//	vec4f quat2 = quatMulVec(&quat, vec);
//	quat2 = quatMulQuat(&quat2, &quatInvert(quat));
//
//	*vec = quat2.xyz;
//
//	//norm = { 1.0f,0,0 };
//	vec3f z = *vec;
//	normalize(&z);
//	rotAxis = cross(&rotAxis, &z);
//	normalize(&rotAxis);
//	halfAngel = angelY * 0.5f;
//	s = sin(halfAngel);
//	quat.w = cos(halfAngel);
//	quat.x = rotAxis.x * s;
//	quat.y = rotAxis.y * s;
//	quat.z = rotAxis.z * s;
//	//quat = preperaQuat(norm, angelY);
//
//	quat2 = quatMulVec(&quat, vec);
//	quat2 = quatMulQuat(&quat2, &quatInvert(quat));
//
//	*vec = quat2.xyz;
//}