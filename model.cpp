#include "model.h"

#define MAX_SIZE 4294967295

#define VERT 1
#define DIFFUSE 2
#define NORMAL 3
#define FACE 4
#define SPECULAR 5

static const char *fileExp[] = {
	"_diffuse",
	"_nm_tangent",
	"_spec"
};

static bool
readFile(LPSTR name, read_file_result *Result)
{
	HANDLE FileHandle = CreateFileA(name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

	if (FileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER FileSize;
		
		if (GetFileSizeEx(FileHandle, &FileSize))
		{
			if (FileSize.QuadPart > MAX_SIZE) return false;
			
			uint32 size32 = (uint32)FileSize.QuadPart;
			Result->memory = VirtualAlloc(0, size32, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

			if (Result->memory)
			{
				DWORD ByteRead;
				if (ReadFile(FileHandle, Result->memory, size32, &ByteRead, 0) &&
					(size32 == ByteRead))
				{
					Result->size = size32;
					return true;
				}
				else
				{
					VirtualFree(Result->memory, 0, MEM_RELEASE);
				}
			}
		}
	}

	return false;
}

static int
readLineVert(read_file_result *Result, char *tmpBuff)
{
	char *readPtr = (char *)Result->memory;
	int j;

	for (int i = 0; readPtr[i] != 0; ++i)
	{
		if ((i == 0 || readPtr[i - 1] == '\n') &&
			(readPtr[i] == 'v' || readPtr[i] == 'f'))
		{
			for (j = 0; readPtr[i] != '\n'; ++j)
			{
				tmpBuff[j] = readPtr[i++];
			}

			tmpBuff[j] = 0;

			Result->memory = (void *)(readPtr + i);
			
			if (tmpBuff[0] == 'f') return FACE;
			else if (tmpBuff[1] == 'n') return NORMAL;
			else if (tmpBuff[1] == 't') return DIFFUSE;
			else return VERT;
		}
	}

	return 0;
}

static void
countUpVertices(model_description *Model, read_file_result *Result)
{
	char *readPtr = (char *)Result->memory;

	int countVert = 0;
	int countNormal = 0;
	int countDiff = 0;
	Model->faceCount = 0;

	for (uint32 i = 0; (readPtr[i] != 0) || (i <= Result->size); ++i)
	{
		if (i == 0 || readPtr[i - 1] == '\n')
		{
			if (readPtr[i] == 'v')
			{
				if (readPtr[i + 1] == 'n')
				{
					++countNormal;
					++i;
				}
				else if (readPtr[i + 1] == 't')
				{
					++countDiff;
					++i;
				}
				else
				{
					++countVert;
				}
			}
			else if (readPtr[i] == 'f')
			{
				++Model->faceCount;
			}
		}
	}

	Model->faceStorage = (int *)VirtualAlloc(
		0, (Model->faceCount * 9) * sizeof(int),
		MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	//vt float float (0)
	Model->vertStorage = (float *)VirtualAlloc(        
		0, (((countVert + countNormal) * 3) + (countDiff * 2)) * sizeof(float),
		MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	Model->normalOffset = countVert;
	Model->diffuseOffset = countVert + countNormal;
	Model->vertSize = Model->diffuseOffset + countDiff;
}

static bool
loadVert(model_description *Model, LPSTR Name)
{
	read_file_result Result;

	if (!readFile(Name, &Result))
	{
		return false;
	}

	countUpVertices(Model, &Result);

	vec3i *faceVert = (vec3i *)Model->faceStorage;
	vec3i *faceNorm = (vec3i *)(Model->faceStorage + NORMAL_OFFSET(Model->faceCount));
	vec3i *faceDiff = (vec3i *)(Model->faceStorage + DIFFUSE_OFFSET(Model->faceCount));

	vec3f *readVert = (vec3f *)Model->vertStorage;
	vec3f *readNorm = (vec3f *)Model->vertStorage + Model->normalOffset;
	vec2f *readDiff = (vec2f *)((vec3f *)Model->vertStorage + Model->diffuseOffset);

	int lineType;
	char lineBuff[128];

	while ((lineType = readLineVert(&Result, lineBuff)))
	{
		vec3i Vert, Diff, Normal;
		vec3f realVert;
		vec2f diffVert;
	
		switch (lineType)
		{
			case VERT:
			{
				sscanf_s(lineBuff, "v %f %f %f", &realVert.x, &realVert.y, &realVert.z);

				//invert coord
#if 0
				realVert.x = -realVert.x;
				realVert.y = -realVert.y;
#endif
				*readVert++ = realVert;
			} break;
			case NORMAL:
			{
				sscanf_s(lineBuff, "vn %f %f %f", &realVert.x, &realVert.y, &realVert.z);
				*readNorm++ = realVert;
			} break;
			case DIFFUSE:
			{
				sscanf_s(lineBuff, "vt %f %f", &diffVert.x, &diffVert.y);

#if 1
				diffVert.x = 1.0f - diffVert.x;
				diffVert.y = 1.0f - diffVert.y;
#endif
				*readDiff++ = diffVert;
			} break;
			case FACE:
			{
				sscanf_s(
					lineBuff, "f %d/%d/%d %d/%d/%d %d/%d/%d",
					&Vert.x, &Diff.x, &Normal.x,
					&Vert.y, &Diff.y, &Normal.y,
					&Vert.z, &Diff.z, &Normal.z);

				Vert -= 1;
				Diff -= 1;
				Normal -= 1;

				*faceVert++ = Vert;
				*faceNorm++ = Normal;
				*faceDiff++ = Diff;
			} break;
		}
	}

	VirtualFree(Result.memory, 0, MEM_RELEASE);

	return true;
}

static char *
getPtrMapName(int typeMap)
{
	switch (typeMap)
	{
		case DIFFUSE:
			return (char *)fileExp[0];
		case NORMAL:
			return (char *)fileExp[1];
		case SPECULAR:
			return (char *)fileExp[2];
		default:
			return 0;

	}
}

static bool
prepareBmpName(char *newName, LPSTR Name, int typeMap, int sizeBuff)
{
	int i;
	char *nameMap = getPtrMapName(typeMap);
	int nameLen = lstrlenA(Name);
	int nameMapLen = lstrlenA(nameMap);

	if ((nameLen + nameMapLen) >= sizeBuff)
	{
		return false;
	}

	for (i = 0; Name[i] != '.'; ++i)
	{
		newName[i] = Name[i];
	}

	for (int j = 0; nameMap[j] != 0; ++j)
	{
		newName[i++] = nameMap[j];
	}

	newName[i++] = '.';
	newName[i++] = 'b';
	newName[i++] = 'm';
	newName[i++] = 'p';
	newName[i] = 0;

	return true;
}

static void
convertByte(void **mapStorage, uint32 sizeMap)
{
	uint32 *pixel = (uint32 *)*mapStorage;

	for (uint32 i = 0; i < sizeMap; i += 4)
	{
		*pixel = (*pixel >> 8) | (*pixel << 24);
		++pixel;
	}
}

static void **
getPtrMap(model_description *Model, int typeMap)
{
	switch (typeMap)
	{
		case DIFFUSE:
			return &Model->diffuseMap;
		case NORMAL:
			return &Model->normalMap;
		case SPECULAR:
			return &Model->specularMap;
		default:
			return 0;
	}
}

static bool
loadBmpMap(model_description *Model, LPSTR Name, int typeMap)
{
	read_file_result Result;
	void **mapStorage;
	char newName[256];
	
	if (!prepareBmpName(newName, Name, typeMap, sizeof(newName)))
	{
		return false;
	}

	if (!readFile(newName, &Result))
	{
		return false;
	}
	
	uint32 bitmapOffset = *(uint32 *)((uint8 *)Result.memory + 10);
	uint32 sizeHeader = *(uint32 *)((uint8 *)Result.memory + 14);
	int32 widthMap = *(uint32 *)((uint8 *)Result.memory + 18);
	int32 heightMap = *(uint32 *)((uint8 *)Result.memory + 22);
	uint16 bitPerPixel = *(uint16 *)((uint8 *)Result.memory + 28);
	int sizeMap = Result.size - sizeHeader;

	//top-down order
	if (/*(heightMap < 0) && */bitPerPixel == 32)
	{
		heightMap = -heightMap;
		if (!Model->width && !Model->height)
		{
			Model->width = widthMap;
			Model->height = heightMap;
		}
		else if (Model->width != widthMap && Model->height != heightMap)
		{
			//all map must be same size
			VirtualFree(Result.memory, 0, MEM_RELEASE);
			return false;
		}
	}
	else
	{
		VirtualFree(Result.memory, 0, MEM_RELEASE);
		return false;
	}

	mapStorage = getPtrMap(Model, typeMap);
	if (mapStorage)
	{
		*mapStorage = (void *)((uint8 *)Result.memory + bitmapOffset);
	}
	else
	{
		VirtualFree(Result.memory, 0, MEM_RELEASE);
		return false;
	}

#if ORDER_BYTE
	if (typeMap != SPECULAR)
	{
		convertByte(mapStorage, sizeMap);
	}
#endif

	return true;
}

uint8
prepareModel(model_description *Model, LPSTR Name)
{
	if (loadVert(Model, Name))
	{
		Model->inclusiveParam |= VERT_SET;

		if (loadBmpMap(Model, Name, DIFFUSE))
		{
			Model->inclusiveParam |= DIFFUSE_SET;
		}

		if (loadBmpMap(Model, Name, NORMAL))
		{
			Model->inclusiveParam |= NORMAL_SET;
		}

		/*if (loadBmpMap(Model, Name, SPECULAR))
		{
			Model->inclusiveParam |= SPECULAR_SET;
		}*/
	}

	return Model->inclusiveParam;
}

void
getNormal(model_description *Model, vec3f *vertArr, int i)
{
	vec3i vecOffset = *(vec3i *)((vec3i *)(Model->faceStorage + NORMAL_OFFSET(Model->faceCount)) + i);
	vertArr[0] = *(vec3f *)((vec3f *)Model->vertStorage + vecOffset.x);
	vertArr[1] = *(vec3f *)((vec3f *)Model->vertStorage + vecOffset.y);
	vertArr[2] = *(vec3f *)((vec3f *)Model->vertStorage + vecOffset.z);
}

void
getDiffuse(model_description *Model, vec2f *text, int i)
{
	vec2f *diffPtr = (vec2f *)((vec3f *)Model->vertStorage + Model->diffuseOffset);
	vec3i vecOffset = *(vec3i *)((vec3i *)(Model->faceStorage + DIFFUSE_OFFSET(Model->faceCount)) + i);

	for (int i = 0; i < 3; i++)
	{
		text[i] = *(diffPtr + vecOffset.elem[i]);
	}
}