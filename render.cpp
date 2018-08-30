#include <cstdlib>
#include <float.h>
#include "render.h"
#include "drawFunction.h"

static draw_line_function *funcPtr[] = {
	defaultLine,
	lineWithTexture,
	lineWithNormal
};

static inline void
swapInt(int &i, int &j)
{
	int tmp = i;
	i = j;
	j = tmp;
}

static inline void
swapVal(float *arr, int order)
{
	vec3f tmp;
	tmp.x = arr[(order & 0xF)];
	tmp.y = arr[(order & 0xF0) >> 4];
	tmp.z = arr[(order & 0xF00) >> 8];

	*(vec3f *)arr = tmp;
}

static inline void
swapVec(vec3f *vec, int a, int b)
{
	vec3f tmp = vec[a];
	vec[a] = vec[b];
	vec[b] = tmp;
}

static float
calcStep(float *var, vec3f *vert, float oneOver, int axis)
{

	float r1 = (var[1] - var[2]) * (vert[0].elem[axis] - vert[2].elem[axis]);
	float r2 = (var[0] - var[2]) * (vert[1].elem[axis] - vert[2].elem[axis]);

	return (r1 - r2) * oneOver;
}

static void
calcIts(float *its, rasterization_info *RasterInfo, model_description *Model, render_state *State)
{
	vec4f dotResult;
	vec4f normalBuff;
	
	int shift = RasterInfo->secondVert - 1;

	for (
		int i = 0, j = 0;
		i <= RasterInfo->secondVert + 1;
		i++, j++)
	{
		normalBuff.w = 1.0f;
		normalBuff.xyz = RasterInfo->normal[i];
		normalBuff = Model->transform * normalBuff;
		dotResult.elem[j] = dot(&normalBuff.xyz, &State->light);

		i += shift;
		shift -= shift;
	}

	dotResult.xmm = _mm_max_ps(dotResult.xmm, _mm_set_ps1(0.0f));
	dotResult.xmm = _mm_min_ps(dotResult.xmm, _mm_set_ps1(1.0f));
	
	*(vec3f *)its = dotResult.xyz;
}

static inline float
calcXStep(vec3f min, vec3f max)
{
	return (max.x - min.x) / (max.y - min.y);
}

static inline void
calcRStep(rasterization_info *RasterInfo, int min, int max)
{
	RasterInfo->Step.xRStep = calcXStep(RasterInfo->vert[min], RasterInfo->vert[max]);
	float yP = ownCeil(RasterInfo->vert[min].y) - RasterInfo->vert[min].y;
	RasterInfo->EdgeStep.rightX = RasterInfo->vert[min].x + yP * RasterInfo->Step.xRStep;
}

#if 0
void
calcTBN(rasterization_info *RasterInfo, model_description *Model, render_state *State)
{
	vec3f deltaPos1 = RasterInfo->tmpvert[1] - RasterInfo->tmpvert[0];
	vec3f deltaPos2 = RasterInfo->tmpvert[2] - RasterInfo->tmpvert[0];

	vec2f deltaUV1, deltaUV2;

	deltaUV1.x = RasterInfo->textureX[1] - RasterInfo->textureX[0];
	deltaUV1.y = RasterInfo->textureY[1] - RasterInfo->textureY[0];

	deltaUV2.x = RasterInfo->textureX[2] - RasterInfo->textureX[0];
	deltaUV2.y = RasterInfo->textureY[2] - RasterInfo->textureY[0];

	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	vec4f tangent, norm;

	norm.w = 0;
	norm.xyz = RasterInfo->normal[0];

	tangent.w = 0;
	tangent.xyz = ((deltaPos1 * deltaUV2.y) - (deltaPos2 * deltaUV1.y)) * f;
	normalize(&tangent.xyz);

	tangent = Model->transform * tangent;
	norm = Model->transform * norm;

	normalize(&tangent);
	normalize(&norm);

	vec3f biTangent = cross(&norm.xyz, &tangent.xyz);
	normalize(&biTangent);

	mat3 *tbn = &RasterInfo->TBN;

	*(vec3f *)tbn = tangent.xyz;
	*((vec3f *)tbn + 1) = biTangent;
	*((vec3f *)tbn + 2) = norm.xyz;
	//transpose(tbn);

	//RasterInfo->tangentLight = RasterInfo->TBN * State->light;
}
#endif

static void
prepareFirstPartStep(
	model_description *Model,
	render_state *State,
	rasterization_info *RasterInfo,
	int orderChange)
{
	float oneOverX = 1.0f / 
		(((RasterInfo->vert[1].x - RasterInfo->vert[2].x) *
		(RasterInfo->vert[0].y - RasterInfo->vert[2].y)) -
		((RasterInfo->vert[0].x - RasterInfo->vert[2].x) *
		(RasterInfo->vert[1].y - RasterInfo->vert[2].y)));

	float oneOverY = -oneOverX;

	float tmpArr[3];
	tmpArr[0] = RasterInfo->vert[0].z;
	tmpArr[1] = RasterInfo->vert[1].z;
	tmpArr[2] = RasterInfo->vert[2].z;

	RasterInfo->Step.depthX = calcStep(tmpArr, RasterInfo->vert, oneOverX, 1);
	RasterInfo->Step.depthY = calcStep(tmpArr, RasterInfo->vert, oneOverY, 0);

	if (!RasterInfo->normalSet)
	{
		calcIts(tmpArr, RasterInfo, Model, State);
		
		if (orderChange != 0x210)
		{
			swapVal(tmpArr, orderChange);
		}

		RasterInfo->Step.light[0] = tmpArr[0];
		RasterInfo->Step.light[1] = tmpArr[1];

		RasterInfo->Step.lightX = calcStep(tmpArr, RasterInfo->vert, oneOverX, 1);
		RasterInfo->Step.lightY = calcStep(tmpArr, RasterInfo->vert, oneOverY, 0);
	}

	//tmp ||
	if (RasterInfo->textureSet || RasterInfo->normalSet)
	{
		RasterInfo->textureX[0] = RasterInfo->clipTexture[0].x;
		RasterInfo->textureY[0] = RasterInfo->clipTexture[0].y;

		for (int i = 1, j = RasterInfo->secondVert; i < 3; i++, j++)
		{
			RasterInfo->textureX[i] = RasterInfo->clipTexture[j].x;
			RasterInfo->textureY[i] = RasterInfo->clipTexture[j].y;
		}

		if (orderChange != 0x210)
		{
			swapVal(RasterInfo->textureX, orderChange);
			swapVal(RasterInfo->textureY, orderChange);
		}

		RasterInfo->Step.textXX = calcStep(RasterInfo->textureX, RasterInfo->vert, oneOverX, 1);
		RasterInfo->Step.textXY = calcStep(RasterInfo->textureX, RasterInfo->vert, oneOverY, 0);
		RasterInfo->Step.textYX = calcStep(RasterInfo->textureY, RasterInfo->vert, oneOverX, 1);
		RasterInfo->Step.textYY = calcStep(RasterInfo->textureY, RasterInfo->vert, oneOverY, 0);
	}
}

static void
prepareSecondPartStep(rasterization_info *RasterInfo, render_state *State, int min, int max)
{
	RasterInfo->Step.xLStep = calcXStep(RasterInfo->vert[min], RasterInfo->vert[max]);
	
	float yPrestep = ownCeil(RasterInfo->vert[min].y) - RasterInfo->vert[min].y;

	RasterInfo->EdgeStep.leftX = RasterInfo->vert[min].x + 
		yPrestep * RasterInfo->Step.xLStep;

	float xPrestep = RasterInfo->EdgeStep.leftX - RasterInfo->vert[min].x;

	RasterInfo->Step.depthH = RasterInfo->Step.depthY + 
		RasterInfo->Step.depthX * RasterInfo->Step.xLStep;

	RasterInfo->EdgeStep.leftZ = RasterInfo->vert[min].z +
		RasterInfo->Step.depthX * xPrestep + RasterInfo->Step.depthY * yPrestep;

	//tmp ||
	if (RasterInfo->textureSet || RasterInfo->normalSet)
	{
		RasterInfo->EdgeStep.textXLeft = RasterInfo->textureX[min] + 
			RasterInfo->Step.textXX * xPrestep + RasterInfo->Step.textXY * yPrestep;

		RasterInfo->EdgeStep.textYLeft = RasterInfo->textureY[min] +
			RasterInfo->Step.textYX * xPrestep + RasterInfo->Step.textYY * yPrestep;

		RasterInfo->Step.textXH = RasterInfo->Step.textXY + 
			RasterInfo->Step.textXX * RasterInfo->Step.xLStep;

		RasterInfo->Step.textYH = RasterInfo->Step.textYY + 
			RasterInfo->Step.textYX * RasterInfo->Step.xLStep;
	}

	if (!RasterInfo->normalSet)
	{
		RasterInfo->EdgeStep.currentLight = RasterInfo->Step.light[min] + 
			RasterInfo->Step.lightX * xPrestep + RasterInfo->Step.lightY * yPrestep;

		RasterInfo->Step.lightH = RasterInfo->Step.lightY +
			RasterInfo->Step.lightX * RasterInfo->Step.xLStep;
	}
}

static void
stepOverY(step_info *Step, edge_step_info *EdgeStep)
{
	__m128 edgeStep = _mm_load_ps((float *)EdgeStep + 4);
	edgeStep = _mm_add_ps(edgeStep, _mm_load_ps((float *)Step + 4));
	_mm_store_ps((float *)EdgeStep + 4, edgeStep);

	EdgeStep->leftX += Step->xLStep;
	EdgeStep->rightX += Step->xRStep;
}

static void
drawEdge(
	rasterization_info *RasterInfo,
	windows_description *Window,
	model_description *Model,
	render_state *State,
	int top)
{
	draw_line_function *drawLine = (draw_line_function *)State->drawLine;
	int yEnd = ownCeil(RasterInfo->vert[top + 1].y);
	float xPrestep = ownCeil(RasterInfo->EdgeStep.leftX) - RasterInfo->EdgeStep.leftX;
	__m128 xPrestep_4x = _mm_set_ps1(xPrestep);

	for (int i = ownCeil(RasterInfo->vert[top].y); i < yEnd; i++)
	{
		__m128 tmp = _mm_mul_ps(xPrestep_4x, _mm_load_ps((float *)&RasterInfo->Step));
		tmp = _mm_add_ps(tmp, _mm_load_ps((float *)&RasterInfo->EdgeStep + 4));
		_mm_store_ps((float *)&RasterInfo->EdgeStep, tmp);

		if (RasterInfo->EdgeStep.leftX != RasterInfo->EdgeStep.rightX)
		{
			drawLine(RasterInfo, Model, Window, State, i);
		}

		stepOverY(&RasterInfo->Step, &RasterInfo->EdgeStep);
	}
	
 	//updateScreen(Window);
}

static void
drawTringe(
	windows_description *Window,
	model_description *Model,
	render_state *State,
	rasterization_info *RasterInfo)
{
	int y0 = (int)RasterInfo->vert[0].y;
	int y1 = (int)RasterInfo->vert[1].y;
	int y2 = (int)RasterInfo->vert[2].y;
	int orderChange = 0x210;

	if (y0 > y1)
	{
		swapInt(y0, y1);
		swapVec(RasterInfo->vert, 0, 1);
		orderChange = 0x201;
	}
	if (y0 > y2)
	{
		swapInt(y0, y2);
		swapVec(RasterInfo->vert, 0, 2);
		orderChange = ((orderChange & 0xFF) << 4) ^ 0x112;
	}
	if (y1 > y2)
	{
		swapVec(RasterInfo->vert, 1, 2);
		orderChange = ((orderChange & 0xF00) >> 4) | ((orderChange & 0xF0) << 4) | (orderChange & 0xF);
	}

	prepareFirstPartStep(Model, State, RasterInfo, orderChange);

	float direction = cross(
		RasterInfo->vert[2].xy - RasterInfo->vert[0].xy,
		RasterInfo->vert[1].xy - RasterInfo->vert[0].xy);

	if (direction >= 0)
	{
		calcRStep(RasterInfo, 0, 2);
		prepareSecondPartStep(RasterInfo, State, 0, 1);
		drawEdge(RasterInfo, Window, Model, State, 0);
		prepareSecondPartStep(RasterInfo, State, 1, 2);
		drawEdge(RasterInfo, Window, Model, State, 1);
	}
	else
	{
		calcRStep(RasterInfo, 0, 1);
		prepareSecondPartStep(RasterInfo, State, 0, 2);
		drawEdge(RasterInfo, Window, Model, State, 0);
		calcRStep(RasterInfo, 1, 2);
		drawEdge(RasterInfo, Window, Model, State, 1);
	}
}

bool
inViewFrustum(vec4f *homCord)
{
	union
	{
		vec4i cmpResultI;
		vec4f cmpResultF;
	};

	int isView = 0xFFFFFFFF;
	__m128 andVal_4x = _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF));

	for (int i = 0; i < 3; i++)
	{
		//abs
		cmpResultF.xmm = _mm_and_ps(homCord[i].xmm, andVal_4x);

		__m128 w_4x = _mm_set_ps1(cmpResultF.w);
		cmpResultF.xmm = _mm_cmple_ps(cmpResultF.xmm, w_4x);

		isView &= cmpResultI.x;
		cmpResultI.y &= cmpResultI.z;
		isView &= cmpResultI.y;

		if (!isView) return false;
	}

	return true;
}

static void
clipPoligonAxisVert(
	clip_vert_info *ClipInfo, vec4f *vert, vec4f *result,
	int axis, float axisFactor, int lastElem)
{
	vec4f previous = vert[lastElem];
	float previousComponent = previous.elem[axis] * axisFactor;
	bool previousInside = previousComponent <= previous.w;
	ClipInfo->count = ClipInfo->vertLert = ClipInfo->vertInside = 0;

	for (int i = 0; i <= lastElem; i++)
	{
		float currentComponent = vert[i].elem[axis] * axisFactor;
		bool currentInside = currentComponent <= vert[i].w;

		if (currentInside ^ previousInside)
		{
			float tmpVal = previous.w - previousComponent;
			float lerp = tmpVal / (tmpVal - (vert[i].w - currentComponent));

			__m128 lerp_4x = _mm_set_ps1(lerp);

			__m128 a = _mm_sub_ps(vert[i].xmm, previous.xmm);
			a = _mm_mul_ps(a, lerp_4x);
			a = _mm_add_ps(a, previous.xmm);

			_mm_store_ps((float *)result, a);
			result++;

			ClipInfo->count++;
			ClipInfo->vertLert |= 0x1 << i;
			ClipInfo->lerpVal[i] = lerp;
		}

		if (currentInside)
		{
			ClipInfo->count++;
			ClipInfo->vertInside |= 0x1 << i;
			*result++ = vert[i];
		}

		previous = vert[i];
		previousComponent = currentComponent;
		previousInside = currentInside;
	}
}

static void
clipPoligonAxisTexture(clip_vert_info *ClipInfo, vec2f *vert, vec2f *result, int lastElem)
{
	vec2f previousVec = vert[lastElem];
	vec2f calcVec;
	int count = 0;

	for (int i = 0; i <= lastElem; i++)
	{
		if (ClipInfo->vertLert & (1 << i))
		{
			calcVec = vert[i] - previousVec;
			calcVec *= ClipInfo->lerpVal[i];
			calcVec += previousVec;

			result[count++] = calcVec;
		}

		if (ClipInfo->vertInside & (1 << i))
		{
			result[count++] = vert[i];
		}

		previousVec = vert[i];
	}
}

static void
clipPoligonAxisNormal(clip_vert_info *ClipInfo, vec3f *vert, vec3f *result, int lastElem)
{
	vec4f previous;
	vec4f calcVec;
	int count = 0;

	previous.xyz = vert[lastElem];

	for (int i = 0; i <= lastElem; i++)
	{
		if (ClipInfo->vertLert & (1 << i))
		{
			calcVec.xyz = vert[i];
			calcVec -= previous;
			calcVec *= ClipInfo->lerpVal[i];
			calcVec += previous;

			result[count++] = calcVec.xyz;
		}

		if (ClipInfo->vertInside & (1 << i))
		{
			result[count++] = vert[i];
		}

		previous.xyz = vert[i];
	}
}

static bool
clipPoligonAxis(rasterization_info *RasterInfo, void *buff, int axis)
{
	clip_vert_info ClipPos;
	clip_vert_info ClipNeg;

	clipPoligonAxisVert(
		&ClipPos, RasterInfo->clipVert, (vec4f *)buff,
		axis, 1.0f, (int)RasterInfo->clipVertLastIndex);

	if (!ClipPos.count) return false;
	ClipPos.count--;

	clipPoligonAxisVert(
		&ClipNeg, (vec4f *)buff, RasterInfo->clipVert,
		axis, -1.0f, ClipPos.count);

	if (!ClipNeg.count) return false;
	ClipNeg.count--;

	//tmp ||
	if (RasterInfo->textureSet || RasterInfo->normalSet)
	{
		clipPoligonAxisTexture(&ClipPos, RasterInfo->clipTexture, (vec2f *)buff, (int)RasterInfo->clipVertLastIndex);
		clipPoligonAxisTexture(&ClipNeg, (vec2f *)buff, RasterInfo->clipTexture, ClipPos.count);
	}

	if (!RasterInfo->normalSet)
	{
		clipPoligonAxisNormal(&ClipPos, RasterInfo->normal, (vec3f *)buff, (int)RasterInfo->clipVertLastIndex);
		clipPoligonAxisNormal(&ClipNeg, (vec3f *)buff, RasterInfo->normal, ClipPos.count);
	}

	RasterInfo->clipVertLastIndex = (uint8)ClipNeg.count;
	return true;
}

static bool
prepareVert(rasterization_info *RasterInfo, render_state *State, mat4 *proj)
{
	for (int i = 0; i < 3; i++)
	{
		RasterInfo->clipVert[i].w = 1.0f;
		RasterInfo->clipVert[i] = *proj * RasterInfo->clipVert[i];

		//RasterInfo->clipVert[i].w = 1.0f;
		//RasterInfo->clipVert[i] = Model->transform * RasterInfo->clipVert[i];
		//RasterInfo->tmpvert[i] = RasterInfo->clipVert[i].xyz;
		//RasterInfo->clipVert[i] = State->proj * RasterInfo->clipVert[i];
	}

	if (!inViewFrustum(RasterInfo->clipVert))
	{
		int buff[24]; 

		/*bool r1 = clipPoligonAxis(RasterInfo, (void *)buff, 0);
		bool r2 = clipPoligonAxis(RasterInfo, (void *)buff, 1);
		bool r3 = clipPoligonAxis(RasterInfo, (void *)buff, 2);*/

 		if (!clipPoligonAxis(RasterInfo, (void *)buff, 0) ||
			!clipPoligonAxis(RasterInfo, (void *)buff, 1) ||
			!clipPoligonAxis(RasterInfo, (void *)buff, 2))
		{
			return false;
		}
	}

	RasterInfo->clipVert[0] = State->viewPort * RasterInfo->clipVert[0];
	RasterInfo->clipVert[0] /= RasterInfo->clipVert[0].w;

	return true;
}

static void
drawModel(windows_description *Window, model_description *Model, render_state *State)
{
	mat4 currentProj = State->proj * Model->transform;
	rasterization_info RasterInfo;
	
	RasterInfo.textureSet = State->renderParam & DIFFUSE_SET;
	RasterInfo.normalSet = State->renderParam & NORMAL_SET;

	for (int i = 0; i <= Model->faceCount; i++)
	{
		RasterInfo.clipVertLastIndex = 2;

		getVertices(Model, RasterInfo.clipVert, i);
		getNormal(Model, RasterInfo.normal, i);

		//tmp ||
		if (RasterInfo.textureSet || RasterInfo.normalSet)
		{
			getDiffuse(Model, RasterInfo.clipTexture, i);
		}

		if (prepareVert(&RasterInfo, State, &currentProj))
		{
			for (int j = 1; j < RasterInfo.clipVertLastIndex; j++)
			{
				vec4f tmp;
				RasterInfo.vert[0] = RasterInfo.clipVert[0].xyz;
				
				for (int k = 1, n = j; k < 3; k++, n++)
				{
					tmp = State->viewPort * RasterInfo.clipVert[n];
					tmp /= tmp.w;
					RasterInfo.vert[k] = tmp.xyz;
				}

				RasterInfo.secondVert = j;
				drawTringe(Window, Model, State, &RasterInfo);
			}
		}
	}
}

void 
initRender(windows_description *Window, model_description *Model)
{
	render_state State = {};

	//TODO: do correct projection view(setLookAt, prespective)
	State.proj = State.viewPort = State.prespective = identity();
	State.running = true;
	State.drawLine = (void *)defaultLine;
	State.renderParam = 0x1;
	State.pos = { 0.0f, 0.0f, 2.0f };
	//State.quatRotate.w = 1.0f;
	State.light = {0, 0, 1.0f};
	State.dir = {0, 0, -1.0f };
	State.yaw = -90.0f * PI / 180.0f;

	float width = (float)(Window->Screen.width - 1);
	float height = (float)(Window->Screen.height - 1);

	screenSpaceTransform(&State.viewPort, width / 2.0f, height / 2.0f);
	setPerspective(&State.prespective, (45.0f * PI / 180.0f), width / height, 0.1f, 100.0f);
	//setViewProgection(&State.proj, &State.prespective, &State.quatRotate, &State.pos);
	//
	vec3f sum = State.pos + State.dir;
	setLookAtLH(&State.proj, &State.prespective, &State.pos, &sum);
	//
	setModelTransformation(&Model->transform, &Model->quatRotate, &Model->pos);

	drawModel(Window, Model, &State);
	updateScreen(Window);

	while (State.running)
	{
		receiveInput((void *)&State, Model);
		drawModel(Window, Model, &State);
		updateScreen(Window);
	}
}

static void
moveVecTowardsWithQuat(vec4f* quat, vec3f *pos, vec3f *dir, float step)
{
	vec4f cunj = conjugate(*quat);
	vec4f tempQuat = quatMulVec(quat, dir);
	tempQuat = quatMulQuat(&tempQuat, &cunj);
	tempQuat *= step;
	*pos += tempQuat.xyz;
}

static void
moveVecTowards(vec3f *pos, vec3f *dir, uint32 vkCode, float step)
{
	vec3f up = { 0, 1.0f, 0 };
	vec3f shift;

	switch (vkCode)
	{
	case 'W':
	{
		shift = *dir * step;
		*pos += shift;
	} break;
	case 'S':
	{
		shift = *dir * step;
		*pos -= shift;
	} break;
	case 'A':
	{
		shift = cross(dir, &up);
		normalize(&shift);
		shift *= step;
		*pos += shift;
	} break;
	case 'D':
	{
		shift = cross(dir, &up);
		normalize(&shift);
		shift *= step;
		*pos -= shift;
	} break;
	}
}

void
processMouse(render_state *State, model_description *Model, int mouseX, int mouseY)
{
#define ROTATE_SPEED 0.0007f

	int16 shiftX = State->mouseX - mouseX;
	int16 shiftY = mouseY - State->mouseY;

	State->mouseX = mouseX;
	State->mouseY = mouseY;

	float sensitivity = ROTATE_SPEED * PI;
	float angelX = sensitivity * (float)shiftX;
	float angelY = sensitivity * (float)shiftY;

	if (State->LButtonPress)
	{
		State->yaw += angelX;
		State->pitch -= angelY;

		rotateTransform2(&State->dir, State->yaw, State->pitch);
		vec3f sum = State->pos + State->dir;
		setLookAtLH(&State->proj, &State->prespective, &State->pos, &sum);

		//rotateTransformQuat(&State->quatRotate, angelX, angelY);
		//setViewProgection(&State->proj, &State->prespective, &State->quatRotate, &State->pos);
	}
	else if (State->RButtonPress)
	{
		rotateTransformQuat(&Model->quatRotate, angelX, angelY);
		setModelTransformation(&Model->transform, &Model->quatRotate, &Model->pos);
	}

#if 1
	char buff[128] = {};
	sprintf_s(
		buff, sizeof(buff), "c:%ff, %ff, %ff\n",
		State->dir.x, State->dir.y, State->dir.z);
	OutputDebugStringA(buff);
#endif
}

void
processKeyboard(render_state *State, model_description *Model, uint32 vkCode, bool press)
{
	switch (vkCode)
	{
		case 'W':
		case 'S':
		case 'A':
		case 'D':
		{
			if (!press)
			{
				float move = 0.09f;

				moveVecTowards(&State->pos, &State->dir, vkCode, move);
				vec3f sum = State->pos + State->dir;
				setLookAtLH(&State->proj, &State->prespective, &State->pos, &sum);

				//moveVecTowardsWithQuat(&State->quatRotate, &State->pos, &dir, move);
				//setViewProgection(&State->proj, &State->prespective, &State->quatRotate, &State->pos);
			}
		} break;
		case 'P':
		{
			if (press)
			{
				if (Model->inclusiveParam & DIFFUSE_SET)
				{
					State->renderParam ^= DIFFUSE_SET;
					uint8 id = (State->renderParam << 1);
					id >>= 2;
					State->drawLine = (void *)funcPtr[id];
				}
			}
		} break;
		case 'O':
		{
			//not working
			/*if (press)
			{
				if (Model->inclusiveParam & NORMAL_SET)
				{
					State->renderParam ^= NORMAL_SET;
					uint8 id = (State->renderParam << 1);
					id >>= 2;
					State->drawLine = (void *)funcPtr[id];
				}
			}*/
		} break;
		case 'L':
		{
			if (press)
			{
				State->renderParam ^= LIGHT_ON;
			}
		} break;
	}

#if 1
	char buff[128] = {};
	sprintf_s(buff, sizeof(buff), "c:%ff, %ff, %ff\n", State->pos.x, State->pos.y, State->pos.z);
	OutputDebugStringA(buff);
#endif
}

void
processMouseWeel(render_state *State, model_description *Model, bool dir)
{
	float angelX = 0.2f;
	angelX = dir ? angelX : -angelX;

	rotateTransform(&State->light, angelX, 0);
	normalize(&State->light);
}