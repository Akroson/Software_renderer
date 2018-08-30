#pragma once

#include "rmath.h"
#include "win32.h"
#include "model.h"

#define LIGHT_ON 0x80

struct edge_step_info
{
	float light;// 2
	float depth;// 2
	float textX;// 2
	float textY;// 2
	float currentLight;//1 3
	float leftZ;//1 3
	float textXLeft;//1 3
	float textYLeft;//1 3
	float leftX;// 3
	float rightX;// 3
};

struct step_info
{
	float lightX;//1 2
	float depthX;//1 2
	float textXX;//1 2
	float textYX;//1 2
	float lightH;// 3
	float depthH;// 3
	float textXH;// 3
	float textYH;// 3
	float xLStep;// 3
	float xRStep;// 3
	float textXY;
	float textYY;
	float depthY;
	float lightY;
	float light[2];
};

struct rasterization_info
{
	step_info Step;
	vec3f vert[3];
	float textureX[3];
	float textureY[3];
	uint8 secondVert;
	uint8 clipVertLastIndex;
	bool textureSet;
	bool normalSet;
	vec4f clipVert[6];
	vec2f clipTexture[6];
	edge_step_info EdgeStep;
	vec3f normal[6];


	//mat3 TBN;
	//vec3f tangentLight;
	//vec3f tmpvert[3];
	//int x;
};

struct render_state
{
	mat4 prespective;
	mat4 viewPort;
	mat4 proj;
	void *drawLine;
#if 0
	vec4f quatRotate;
	vec3f pos;
#else
	vec3f dir;
	vec3f pos;
	float yaw;
	float pitch;
#endif
	vec3f light;
	int16 mouseX, mouseY;
	uint8 renderParam;
	bool running;
	bool LButtonPress;
	bool RButtonPress;
};

#define DRAW_LINE_FUNCTION(name) void name(\
	rasterization_info *RasterInfo,\
	model_description *Model,\
	windows_description *Window,\
	render_state *State,\
	int i)

typedef DRAW_LINE_FUNCTION(draw_line_function);

struct clip_vert_info
{
	float lerpVal[6];
	int count;
	uint16 vertLert;
	uint16 vertInside;
};

struct pixelColor
{
	union
	{
		struct
		{
			uint8 b;
			uint8 g;
			uint8 r;
			uint8 a;
		};

		int rgb;
	};
};

void 
initRender(windows_description *Window, model_description *Model);

void
processMouse(render_state *State, model_description *Model, int mouseX, int mouseY);

void
processKeyboard(render_state *State, model_description *Model, uint32 vkKey, bool press);

void
processMouseWeel(render_state *State, model_description *Model, bool dir);

inline int
ownCeil(float a)
{
	return (int)(a + 1.0f);
}