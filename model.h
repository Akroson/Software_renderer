#pragma once
#include <windows.h>
#include <stdio.h>
#include "rmath.h"

#define VERT_SET 0x1
#define DIFFUSE_SET 0x2
#define NORMAL_SET 0x4
#define SPECULAR_SET 0x8

#define NORMAL_OFFSET(faceCount) (faceCount * 3)
#define DIFFUSE_OFFSET(faceCount) (faceCount * 6)

struct read_file_result
{
	void* memory;
	uint32 size;
};

struct model_description
{
	mat4 transform;
	int *faceStorage;
	float *vertStorage;
	void *diffuseMap;
	void *normalMap;
	void *specularMap;
	int faceCount;
	int normalOffset;
	int diffuseOffset;
	int vertSize;
	uint16 width;
	uint16 height;
	uint8 inclusiveParam;
	vec4f quatRotate;
	vec3f pos;
};

void
getNormal(model_description *Model, vec3f *vertArr, int i);

void
getDiffuse(model_description *Model, vec2f *text, int i);

uint8
prepareModel(model_description *Model, LPSTR Name);

inline void
getVertices(model_description *Model, vec4f *vertArr, int i)
{
	vec3i face = *(vec3i *)((vec3i *)Model->faceStorage + i);
	vertArr[0].xyz = *(vec3f *)((vec3f *)Model->vertStorage + face.x);
	vertArr[1].xyz = *(vec3f *)((vec3f *)Model->vertStorage + face.y);
	vertArr[2].xyz = *(vec3f *)((vec3f *)Model->vertStorage + face.z);
}

inline int
getDiffuseFromMap(model_description *Model, int x, int y)
{
	int offset = x + (y * Model->width);
	return *(uint32 *)((uint32 *)Model->diffuseMap + offset);
}

inline int
getNormalFromMap(model_description *Model, int x, int y)
{
	int offset = x + (y * Model->width);
	return *(uint32 *)((uint32 *)Model->normalMap + offset);
}